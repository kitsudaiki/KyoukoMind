﻿/**
 *  @file    processing_unit.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/processing/processing_unit.h>
#include <core/brick_handler.h>
#include <root_object.h>

#include <core/global_values_handler.h>

#include <core/objects/brick.h>
#include <core/objects/container_definitions.h>

#include <core/processing/processing_methods/container_processing_methods.h>
#include <core/processing/processing_methods/brick_processing_methods.h>
#include <core/processing/processing_methods/neighbor_methods.h>

#include <libKitsunemimiPersistence/logger/logger.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingUnit::ProcessingUnit
 * @param brickHandler
 */
ProcessingUnit::ProcessingUnit()
{
    m_block = true;
}

/**
 * @brief ProcessingUnit::run
 */
void
ProcessingUnit::run()
{
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;

    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }

        Brick* brick = RootObject::m_brickHandler->getFromQueue();
        if(brick == nullptr)
        {
            GlobalValues globalValues = RootObject::m_globalValuesHandler->getGlobalValues();
            globalValues.globalLearningTemp = 0.0f;
            globalValues.globalMemorizingTemp = 0.0f;
            RootObject::m_globalValuesHandler->setGlobalValues(globalValues);

            end = std::chrono::system_clock::now();
            const float duration = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
            LOG_DEBUG("time: " + std::to_string(duration / 1000.0f) + '\n');

            // block thread until next cycle if queue is empty
            blockThread();

            start = std::chrono::system_clock::now();
        }
        else
        {
            initCycle(brick);

            // main-processing
            brick->globalValues = RootObject::m_globalValuesHandler->getGlobalValues();
            processIncomingMessages(*brick);
            if(brick->dataConnections[NODE_DATA].inUse == 1) {
                processNodes(*brick, m_weightMap);
            }

            // post-processing
            postLearning(*brick);
            memorizeSynapses(*brick);

            // write output
            writeClientOutput(*brick, m_clientBuffer);
            writeMonitoringOutput(*brick, m_monitoringBuffer);

            // finish current block
            finishCycle(brick,
                        m_clientBuffer,
                        m_monitoringBuffer);
        }
    }
}

/**
 * refillWeightMap fill the weight-map which is required for learing-process
 */
void
ProcessingUnit::refillWeightMap(Brick &brick,
                                const uint8_t initialSide,
                                Neighbor* neighbors)
{
    m_totalWeightMap = 0.0f;

    // set all weights
    for(uint8_t side = 0; side < 22; side++)
    {
        if(neighbors[side].targetBrick != nullptr
                && side != initialSide)
        {
            const float randVal = static_cast<float>(rand() % UNINIT_STATE_16);
            m_totalWeightMap += randVal;
            m_weightMap[side] += randVal;
        }
        else
        {
            m_weightMap[side] = 0.0f;
        }
    }

    // if brick contains nodes, the have to get a big part of the incoming weight
    if(brick.dataConnections[NODE_DATA].inUse == 1)
    {
        // TODO: replace the const-value of 0.2f by define
        const float val = m_totalWeightMap * 0.2f;
        m_weightMap[22] = val;
        m_totalWeightMap += val;
    }
    else
    {
        m_weightMap[22] = 0.0f;
    }

    // recalc values
    m_totalWeightMap = 1.0f / m_totalWeightMap;
    for(uint8_t side = 0; side < 23; side++)
    {
        m_weightMap[side] *= m_totalWeightMap;
    }
}

/**
 * @brief ProcessingUnit::processIncomingMessages
 * @param brick
 * @return
 */
void
ProcessingUnit::processIncomingMessages(Brick &brick)
{
    // process normal communication
    for(uint8_t side = 0; side < 23; side++)
    {
        if(brick.neighbors[side].inUse == 1)
        {
            refillWeightMap(brick, side, brick.neighbors);

            StackBuffer* currentBuffer = getCurrentBuffer(brick.neighbors[side]);
            DataBuffer* currentBlock = getFirstElement(*currentBuffer);

            while(currentBlock != nullptr)
            {
                processIncomingMessage(brick, side, currentBlock);
                removeFirstFromStack(*currentBuffer);
                currentBlock = getFirstElement(*currentBuffer);
            }
        }
    }
}

/**
 * processing of all incoming messages in a brick
 *
 * @return false if a message-type does not exist, else true
 */
bool
ProcessingUnit::processIncomingMessage(Brick &brick,
                                       const uint8_t side,
                                       DataBuffer* message)
{
    bool result = true;

    // get start and end of the message-payload
    uint8_t* data = static_cast<uint8_t*>(message->data);
    uint8_t* end = data + message->bufferPosition;

    while(data < end)
    {
        const uint8_t type = data[0];
        void* obj = static_cast<void*>(data);

        switch(type)
        {
            // -------------------------------------------------------------------------------------
            case STATUS_EDGE_CONTAINER:
            {
                UpdateEdgeContainer* edge = static_cast<UpdateEdgeContainer*>(obj);
                data += sizeof(UpdateEdgeContainer);

                assert(edge->targetId != UNINIT_STATE_32);
                processUpdateEdge(brick,
                                  edge->targetId,
                                  edge->updateValue,
                                  edge->updateType,
                                  side);
                break;
            }
            // -------------------------------------------------------------------------------------
            case PENDING_EDGE_CONTAINER:
            {
                PendingEdgeContainer* edge = static_cast<PendingEdgeContainer*>(obj);
                data += sizeof(PendingEdgeContainer);

                assert(edge->sourceEdgeSectionId != UNINIT_STATE_32);
                processPendingEdge(brick,
                                   edge->sourceEdgeSectionId,
                                   edge->sourceSide,
                                   edge->weight,
                                   m_weightMap);
                break;
            }
            // -------------------------------------------------------------------------------------
            case FOREWARD_EDGE_CONTAINER:
            {
                EdgeContainer* edge = static_cast<EdgeContainer*>(obj);
                data += sizeof(EdgeContainer);

                assert(edge->targetEdgeSectionId != UNINIT_STATE_32);
                processEdgeForwardSection(brick,
                                          edge->targetEdgeSectionId,
                                          edge->weight,
                                          m_weightMap);
                break;
            }
            // -------------------------------------------------------------------------------------
            case AXON_EDGE_CONTAINER:
            {
                AxonEdgeContainer* edge = static_cast<AxonEdgeContainer*>(obj);
                data += sizeof(AxonEdgeContainer);

                assert(edge->targetAxonId != UNINIT_STATE_32);
                processAxon(brick,
                            edge->targetAxonId,
                            edge->targetBrickPath,
                            edge->weight,
                            m_weightMap);
                break;
            }
            // -------------------------------------------------------------------------------------
            case LEARNING_EDGE_CONTAINER:
            {
                LearingEdgeContainer* edge = static_cast<LearingEdgeContainer*>(obj);
                data += sizeof(LearingEdgeContainer);

                assert(edge->sourceEdgeSectionId != UNINIT_STATE_32);
                processLerningEdge(brick,
                                   edge->sourceEdgeSectionId,
                                   edge->weight,
                                   side,
                                   m_weightMap);
                break;
            }
            // -------------------------------------------------------------------------------------
            case LEARNING_REPLY_EDGE_CONTAINER:
            {
                LearningEdgeReplyContainer* edge = static_cast<LearningEdgeReplyContainer*>(obj);
                data += sizeof(LearningEdgeReplyContainer);

                EdgeSection* edgeSections = getEdgeBlock(&brick.dataConnections[EDGE_DATA]);
                assert(edge->sourceEdgeSectionId != UNINIT_STATE_32);
                edgeSections[edge->sourceEdgeSectionId].edges[side].targetId =
                        edge->targetEdgeSectionId;
                break;
            }
            // -------------------------------------------------------------------------------------
            case DIRECT_EDGE_CONTAINER:
            {
                DirectEdgeContainer* edge = static_cast<DirectEdgeContainer*>(obj);
                data += sizeof(DirectEdgeContainer);

                Node* nodes = static_cast<Node*>(brick.dataConnections[NODE_DATA].buffer.data);
                Node* node = &nodes[edge->targetNodeId];
                node->currentState = edge->weight;
                break;
            }
            // -------------------------------------------------------------------------------------
            case UNDEFINED_CONTAINER:
            {
                return result;
            }
            // -------------------------------------------------------------------------------------
            default:
                result = false;
        }
    }

    return result;
}

} // namespace KyoukoMind
