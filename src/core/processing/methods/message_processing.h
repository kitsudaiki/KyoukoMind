/**
 *  @file    message_processing.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef EDGE_CONTAINER_PROCESSING_H
#define EDGE_CONTAINER_PROCESSING_H

#include <common.h>
#include <kyouko_root.h>

#include <core/processing/objects/container_definitions.h>
#include <core/processing/methods/synapse_methods.h>
#include <core/methods/data_connection_methods.h>

#include <core/brick.h>
#include <core/network_segment.h>

#include <core/processing/methods/process_normal.h>

namespace KyoukoMind
{

//==================================================================================================

/**
 * processing of all incoming messages in a brick
 *
 * @return false if a message-type does not exist, else true
 */
inline bool
processIncomingMessage(NetworkSegment &segment,
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
                processLearningEdge(segment, brick, edge, side);
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

/**
 * @brief ProcessingUnit::processIncomingMessages
 * @param brick
 * @return
 */
inline void
processIncomingMessages(NetworkSegment &segment,
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

} // namespace KyoukoMind

#endif // EDGE_CONTAINER_PROCESSING_H
