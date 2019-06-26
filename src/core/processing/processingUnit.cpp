/**
 *  @file    processingUnit.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/processing/processingUnit.h>
#include <core/bricks/brickHandler.h>
#include <kyoukoNetwork.h>

#include <core/bricks/globalValuesHandler.h>

#include <core/bricks/brickObjects/brick.h>

#include <core/bricks/brickMethods/commonBrickMethods.h>
#include <core/bricks/brickMethods/bufferControlMethods.h>
#include <core/processing/processingMethods/messageProcessingMethods.h>
#include <core/processing/processingMethods/brickProcessingMethods.h>

#include <core/messaging/messageMarker/outgoingBuffer.h>
#include <core/messaging/messageObjects/contentContainer.h>

#include <core/bricks/brickObjects/brick.h>
#include <core/bricks/brickMethods/bufferControlMethods.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingUnit::ProcessingUnit
 * @param brickHandler
 */
ProcessingUnit::ProcessingUnit()
{
    m_block = true;
    m_monitoringMessage = new TransferDataMessage();
    m_monitoringMessage->init();
    m_clienOutputMessage = new TransferDataMessage();
    m_clienOutputMessage->init();
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
        Brick* brick = KyoukoNetwork::m_brickHandler->getFromQueue();
        if(brick == nullptr)
        {
            GlobalValues globalValues = KyoukoNetwork::m_globalValuesHandler->getGlobalValues();
            globalValues.globalLearningTemp = 0.0f;
            globalValues.globalMemorizingTemp = 0.0f;
            KyoukoNetwork::m_globalValuesHandler->setGlobalValues(globalValues);

            end = std::chrono::system_clock::now();
            const float duration = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
            std::cout << "time: " << (duration / 1000.0f) << '\n';

            // block thread until next cycle if queue is empty
            blockThread();

            start = std::chrono::system_clock::now();
        }
        else
        {
            // main-processing
            assert(brick->isReady() == true);
            brick->globalValues = KyoukoNetwork::m_globalValuesHandler->getGlobalValues();
            processIncomingMessages(brick);
            if(brick->dataConnections[NODE_DATA].inUse == 1) {
                processNodes(brick, m_weightMap);
            }

            // post-processing
            postLearning(brick);
            memorizeSynapses(brick);

            // monitoring
            if(m_enableMonitoring) {
                writeStatus(brick, m_monitoringMessage);
            }

            // client-output
            if(m_enableClient && brick->isOutputBrick) {
                writeOutput(brick, m_clienOutputMessage);
            }

            finishCycle(brick,
                        m_monitoringMessage,
                        m_clienOutputMessage);
        }
    }
}

/**
 * refillWeightMap fill the weight-map which is required for learing-process
 *
 * @param initialSide brick-side where the message comes in
 * @param neighbors pointer to the neighbor-list to check which neighbors are initialized
 * @param weightMap pointer to the map which should be refilled
 */
void
ProcessingUnit::refillWeightMap(Brick* brick,
                                const uint8_t initialSide,
                                Neighbor* neighbors)
{
    uint8_t possibleSides[25];
    uint8_t possibleSidesPos = 0;

    // cleara existing map
    for(uint8_t side = 0; side < 25; side++)
    {
        m_weightMap[side] = 0.0;
    }

    uint8_t runs = 10;

    // get possible next
    for(uint8_t side = 0; side < 24; side++)
    {
        if(neighbors[side].targetBrickId != UNINIT_STATE_32
                && side != initialSide)
        {
            possibleSides[possibleSidesPos] = side;
            possibleSidesPos++;
        }
    }

    if(brick->dataConnections[NODE_DATA].inUse == 1
            && brick->isInputBrick == 0)
    {
        m_weightMap[24] = 0.2;
        possibleSides[possibleSidesPos] = 24;
        possibleSidesPos++;
        runs -= 2;
    }

    if(possibleSidesPos == 0) {
        return;
    }

    // share weights
    for(uint8_t i = 0; i < runs; i++)
    {
        const uint8_t side = possibleSides[rand() % possibleSidesPos];
        m_weightMap[side] += 0.1;
    }
}

/**
 * @brief ProcessingUnit::processIncomingMessages
 * @param brick
 * @return
 */
void
ProcessingUnit::processIncomingMessages(Brick *brick)
{
    // process normal communication
    for(uint8_t side = 0; side < 25; side++)
    {
        if(brick->neighbors[side].inUse == 1)
        {
            refillWeightMap(brick, side, brick->neighbors);

            uint64_t bufferPosition = brick->neighbors[side].incomBuffer.getMessage();
            assert(brick->isReady() == true);

            // skip init-messages
            if(bufferPosition == UNINIT_STATE_64 - 1) {
                continue;
            }
            if(side == 24
                    && bufferPosition == UNINIT_STATE_64)
            {
                continue;
            }
            assert(bufferPosition != UNINIT_STATE_64);

            // pre-check
            DataMessage* message = KyoukoNetwork::m_messageBuffer->getMessage(bufferPosition);
            if(side == 24
                    && message->isLast == 0)
            {
                continue;
            }
            assert(message->isLast == 1);

            // run processing
            while(bufferPosition != UNINIT_STATE_64)
            {
                message = KyoukoNetwork::m_messageBuffer->getMessage(bufferPosition);
                processIncomingMessage(brick, side, message);
                bufferPosition = message->prePosition;
                KyoukoNetwork::m_messageBuffer->finishMessage(message->currentPosition);
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
ProcessingUnit::processIncomingMessage(Brick *brick,
                                       const uint8_t side,
                                       DataMessage* message)
{
    bool result = true;

    // get start and end of the message-payload
    uint8_t* data = message->data;
    uint8_t* end = data + message->size;

    while(data < end)
    {
        const uint8_t type = data[0];
        switch(type)
        {
            // -------------------------------------------------------------------------------------
            case STATUS_EDGE_CONTAINER:
            {
                UpdateEdgeContainer* edge = (UpdateEdgeContainer*)data;
                data += sizeof(UpdateEdgeContainer);

                if(edge->targetId == UNINIT_STATE_32
                        && edge->updateValue >= 0.0f)
                {
                    continue;
                }

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
                PendingEdgeContainer* edge = (PendingEdgeContainer*)data;
                data += sizeof(PendingEdgeContainer);

                if(edge->sourceEdgeSectionId == UNINIT_STATE_32
                        && edge->weight >= 0.0f)
                {
                    continue;
                }

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
                EdgeContainer* edge = (EdgeContainer*)data;
                data += sizeof(EdgeContainer);

                if(edge->targetEdgeSectionId == UNINIT_STATE_32
                        && edge->weight > 0.0f)
                {
                    continue;
                }

                processEdgeForwardSection(brick,
                                          edge->targetEdgeSectionId,
                                          edge->weight,
                                          m_weightMap);
                break;
            }
            // -------------------------------------------------------------------------------------
            case AXON_EDGE_CONTAINER:
            {
                AxonEdgeContainer* edge = (AxonEdgeContainer*)data;
                data += sizeof(AxonEdgeContainer);

                if(edge->targetAxonId == UNINIT_STATE_32 && edge->weight >= 0.0f) {
                    continue;
                }

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
                LearingEdgeContainer* edge = (LearingEdgeContainer*)data;
                data += sizeof(LearingEdgeContainer);

                if(edge->sourceEdgeSectionId == UNINIT_STATE_32
                        && edge->weight >= 0.0f)
                {
                    continue;
                }

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
                LearningEdgeReplyContainer* edge = (LearningEdgeReplyContainer*)data;
                data += sizeof(LearningEdgeReplyContainer);

                EdgeSection* edgeSections = getEdgeBlock(&brick->dataConnections[EDGE_DATA]);

                if(edge->sourceEdgeSectionId == UNINIT_STATE_32) {
                    continue;
                }
                if(edgeSections[edge->sourceEdgeSectionId].edges[side].weight == 0.0f) {
                    continue;
                }

                edgeSections[edge->sourceEdgeSectionId].edges[side].targetId =
                        edge->targetEdgeSectionId;
                break;
            }
            // -------------------------------------------------------------------------------------
            case DIRECT_EDGE_CONTAINER:
            {
                DirectEdgeContainer* edge = (DirectEdgeContainer*)data;
                data += sizeof(DirectEdgeContainer);

                if(brick->isInputBrick == 0
                        || brick->dataConnections[NODE_DATA].inUse == 0)
                {
                    continue;
                }

                Node* nodes = (Node*)brick->dataConnections[NODE_DATA].buffer.data;
                Node* node = &nodes[edge->targetNodeId];
                node->currentState = edge->weight;
                break;
            }
            // -------------------------------------------------------------------------------------
            case UNDEFINED_TYPE:
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

}
