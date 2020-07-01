/**
 *  @file    brick_cycle_methods.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#include "brick_cycle_methods.h"

#include <core/objects/brick.h>
#include <core/objects/container_definitions.h>

#include <core/processing/methods/message_processing.h>
#include <core/methods/brick_item_methods.h>
#include <core/methods/neighbor_methods.h>
#include <core/methods/network_segment_methods.h>

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>

#include <libKitsunemimiKyoukoCommon/communication_structs/monitoring_contianer.h>
#include <libKitsunemimiKyoukoCommon/communication_structs/client_contianer.h>
#include <libKitsunemimiKyoukoCommon/communication_structs/mind_container.h>

namespace KyoukoMind
{

//==================================================================================================

/**
 * @brief initCycle
 * @param brick
 */
void
initCycle(Brick *brick)
{
    while(brick->lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    for(uint8_t side = 0; side < 23; side++) {
        switchNeighborBuffer(brick->neighbors[side]);
    }

    brick->lock.clear(std::memory_order_release);
}

//==================================================================================================

/**
 * @brief finishCycle
 * @param brick
 * @param monitoringMessage
 * @param clientMessage
 */
void
finishCycle(Brick* brick)
{
    while(brick->lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    // finish standard-neighbors
    for(uint8_t side = 0; side < 23; side++) {
        finishSide(brick, side);
    }

    processReady(brick);

    brick->lock.clear(std::memory_order_release);
}

//==================================================================================================

/**
 * @brief finishSide
 * @param brick
 * @param side
 */
void
finishSide(Brick* brick,
           const uint8_t sourceSide)
{
    Neighbor* sourceNeighbor = &brick->neighbors[sourceSide];
    if(sourceNeighbor->inUse == 0) {
        return;
    }

    Brick* targetBrick = sourceNeighbor->targetBrick;

    while(targetBrick->lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    // finish side
    sendNeighborBuffer(*sourceNeighbor, *sourceNeighbor->targetNeighbor);
    processReady(targetBrick);

    targetBrick->lock.clear(std::memory_order_release);
}

//==================================================================================================

/**
 * @brief processReady
 * @param brick
 * @return
 */
bool
processReady(Brick* brick)
{
    if(isReady(brick)
            && brick->inQueue == 0)
    {
        KyoukoRoot::m_queue->addToQueue(brick);
        brick->lock.clear(std::memory_order_release);
        return true;
    }

    return false;
}

//==================================================================================================

/**
 * @brief updateReady
 * @param brick
 * @param side
 */
bool
isReady(Brick* brick)
{
    for(uint8_t side = 0; side < 23; side++)
    {
        if(brick->neighbors[side].inUse == 1
                && brick->neighbors[side].bufferQueue.size() == 0)
        {
            return false;
        }
    }

    return true;
}

//==================================================================================================

/**
 * summarize the state of all nodes in a brick
 * and return the average value of the last two cycles
 * for a cleaner output
 *
 * @return summend value of all nodes of the brick
 */
inline float
getSummedValue(NetworkSegment &segment,
               Brick &brick)
{
    assert(brick.isOutputBrick != 0);
    assert(brick.nodePos >= 0);

    Node* node = &getNodeBlock(segment)[brick.nodePos];
    return node->currentState;

    // write value to the internal ring-buffer
    /*brick.outBuffer[brick.outBufferPos] += node->currentState;
    brick.outBufferPos = (brick.outBufferPos + 1) % 10;
    node->currentState /= NODE_COOLDOWN;

    // summarize the ring-buffer and get the average value
    float result = 0.0f;
    for(uint32_t i = 0; i < 10; i++)
    {
        result += brick.outBuffer[i];
    }
    result /= 10.0f;

    return result;*/
}

//==================================================================================================

/**
 * @brief reportStatus
 */
void
writeMonitoringOutput(Brick &brick,
                      DataBuffer &buffer)
{
    GlobalValues globalValue = KyoukoRoot::m_globalValuesHandler->getGlobalValues();

    // fill message
    Kitsunemimi::Kyouko::MonitoringMessage monitoringMessage;
    monitoringMessage.brickId = brick.brickId;
    monitoringMessage.xPos = brick.brickPos.x;
    monitoringMessage.yPos = brick.brickPos.y;

    // edges
    if(brick.edges.inUse == 1) {
        monitoringMessage.numberOfEdgeSections = brick.edges.numberOfItems
                                                 - brick.edges.numberOfDeletedDynamicItems;
    }

    monitoringMessage.globalLearning = globalValue.globalLearningOffset;
    monitoringMessage.globalMemorizing = globalValue.globalMemorizingOffset;

    Kitsunemimi::addObject_DataBuffer(buffer, &monitoringMessage);
}

//==================================================================================================

/**
 * @brief writeOutput
 * @param brick
 * @param buffer
 */
void
writeClientOutput(NetworkSegment &segment,
                  Brick &brick,
                  DataBuffer &buffer)
{
    Kitsunemimi::Kyouko::MindOutputData outputMessage;
    outputMessage.value = getSummedValue(segment, brick);
    outputMessage.brickId = brick.brickId;

    Kitsunemimi::addObject_DataBuffer(buffer, &outputMessage);
}

//==================================================================================================

} // namespace KyoukoMind
