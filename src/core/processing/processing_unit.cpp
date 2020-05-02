/**
 *  @file    processing_unit.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#include <core/processing/processing_unit.h>
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

    NetworkSegment* segment = RootObject::m_segment;

    while(!m_abort)
    {
        if(m_block) {
            blockThread();
        }

        Brick* brick = RootObject::m_queue->getFromQueue();
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
            processIncomingMessages(*segment, *brick);
            if(brick->nodePos >= 0) {
                processNodes(*segment, *brick);
            }

            // post-processing
            postLearning(*segment, *brick);
            memorizeSynapses(*segment, *brick);

            // write output
            if(brick->isOutputBrick == 1) {
                writeClientOutput(*segment, *brick, m_clientBuffer);
            }
            writeMonitoringOutput(*brick, m_monitoringBuffer);

            // finish current block
            finishCycle(brick,
                        m_clientBuffer,
                        m_monitoringBuffer);
        }
    }
}

/**
 * @brief ProcessingUnit::processIncomingMessages
 * @param brick
 * @return
 */
void
ProcessingUnit::processIncomingMessages(NetworkSegment &segment,
                                        Brick &brick)
{
    // process normal communication
    for(uint8_t side = 0; side < 23; side++)
    {
        if(brick.neighbors[side].inUse == 1)
        {
            StackBuffer* currentBuffer = brick.neighbors[side].currentBuffer;
            DataBuffer* currentBlock = Kitsunemimi::getFirstElement_StackBuffer(*currentBuffer);

            while(currentBlock != nullptr)
            {
                processIncomingMessage(segment, brick, side, currentBlock);
                Kitsunemimi::removeFirst_StackBuffer(*currentBuffer);
                currentBlock = Kitsunemimi::getFirstElement_StackBuffer(*currentBuffer);
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
ProcessingUnit::processIncomingMessage(NetworkSegment &segment,
                                       Brick &brick,
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
                const UpdateEdgeContainer edge = *static_cast<UpdateEdgeContainer*>(obj);
                assert(edge.targetId != UNINIT_STATE_32);
                processUpdateEdge(brick, edge, side);
                data += sizeof(UpdateEdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case PENDING_EDGE_CONTAINER:
            {
                const PendingEdgeContainer edge = *static_cast<PendingEdgeContainer*>(obj);
                assert(edge.sourceEdgeSectionId != UNINIT_STATE_32);
                processPendingEdge(segment, brick, edge);
                data += sizeof(PendingEdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case FOREWARD_EDGE_CONTAINER:
            {
                const EdgeContainer edge = *static_cast<EdgeContainer*>(obj);
                assert(edge.targetEdgeSectionId != UNINIT_STATE_32);
                processEdgeForwardSection(segment, brick, edge);
                data += sizeof(EdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case AXON_EDGE_CONTAINER:
            {
                const AxonEdgeContainer edge = *static_cast<AxonEdgeContainer*>(obj);
                assert(edge.targetAxonId != UNINIT_STATE_32);
                processAxon(segment, brick, edge);
                data += sizeof(AxonEdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case LEARNING_EDGE_CONTAINER:
            {
                const LearingEdgeContainer edge = *static_cast<LearingEdgeContainer*>(obj);
                assert(edge.sourceEdgeSectionId != UNINIT_STATE_32);
                assert(side != 0);
                processLerningEdge(segment, brick, edge, side);
                data += sizeof(LearingEdgeContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case LEARNING_REPLY_EDGE_CONTAINER:
            {
                const LearningEdgeReplyContainer edge =
                        *static_cast<LearningEdgeReplyContainer*>(obj);
                assert(edge.sourceEdgeSectionId != UNINIT_STATE_32);
                processLearningEdgeReply(brick, edge, side);
                data += sizeof(LearningEdgeReplyContainer);
                break;
            }
            // -------------------------------------------------------------------------------------
            case DIRECT_EDGE_CONTAINER:
            {
                const DirectEdgeContainer edge = *static_cast<DirectEdgeContainer*>(obj);
                processDirectEdge(segment, brick, edge);
                data += sizeof(DirectEdgeContainer);
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
