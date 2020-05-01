/**
 *  @file    brick_processing_methods.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "brick_processing_methods.h"

#include <core/objects/brick.h>
#include <core/objects/container_definitions.h>

#include <core/processing/processing_methods/container_processing_methods.h>
#include <core/processing/processing_methods/brick_item_methods.h>
#include <core/processing/processing_methods/neighbor_methods.h>

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

    for(uint8_t side = 0; side < 23; side++)
    {
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
finishCycle(Brick* brick,
            DataBuffer &clientMessage,
            DataBuffer &monitoringMessage)
{
    while(brick->lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    // finish standard-neighbors
    for(uint8_t side = 0; side < 23; side++)
    {
        finishSide(brick, side);
    }

    processReady(brick);

    if(RootObject::m_clientSession != nullptr
            && brick->isOutputBrick == 1)
    {
        RootObject::m_clientSession->sendStreamData(clientMessage.data,
                                                    clientMessage.bufferPosition);
    }

    if(RootObject::m_monitoringSession != nullptr) {
        RootObject::m_monitoringSession->sendStreamData(monitoringMessage.data,
                                                        monitoringMessage.bufferPosition);
    }

    clientMessage.bufferPosition = 0;
    monitoringMessage.bufferPosition = 0;

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

    assert(targetBrick != nullptr);

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
        RootObject::m_brickHandler->addToQueue(brick);
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
 * processing of the nodes of a specific node-brick
 *
 * @return number of active nodes in this brick
 */
uint16_t
processNodes(NetworkSegment &segment,
             Brick &brick)
{
    if(brick.isOutputBrick != 0) {
        return 0;
    }

    uint16_t numberOfActiveNodes = 0;

    // process nodes
    Node* start = &getNodeBlock(segment.nodes)[brick.nodePos];
    Node* end = start + NUMBER_OF_NODES_PER_BRICK;

    // iterate over all nodes in the brick
    for(Node* node = start;
        node < end;
        node++)
    {
        // set to 255.0f, if value is too high
        if(node->currentState > 255.0f) {
            node->currentState = 255.0f;
        }

        // init
        const Node tempNode = *node;

        // check if active
        if(tempNode.border <= tempNode.currentState
                && tempNode.refractionTime == 0)
        {
            node->potential = ACTION_POTENTIAL;
            numberOfActiveNodes++;
            node->active = 1;
            node->refractionTime = REFRACTION_TIME;
        }
        else if(tempNode.refractionTime == 0) {
            node->active = 0;
        }

        // forward current potential
        if(node->active == 1)
        {
            const float weight = node->potential * PROCESSING_MULTIPLICATOR;
            const uint64_t path = tempNode.targetBrickPath/32;

            AxonEdgeContainer newEdge;
            newEdge.targetAxonId = tempNode.targetAxonId;
            newEdge.targetBrickPath = path;
            newEdge.weight = weight;

            processAxon(segment, brick, newEdge);
        }
        // post-steps
        node->refractionTime = node->refractionTime >> 1;

        // set to 0.0f, if value is negative
        if(node->currentState < 0.0f) {
            node->currentState = 0.0f;
        }

        // check if node-state is too high compared to the border
        node->tooHigh = node->currentState > 1.2f * node->border;

        // make cooldown in the node
        node->potential /= NODE_COOLDOWN;
        node->currentState /= NODE_COOLDOWN;
    }

    return numberOfActiveNodes;
}

//==================================================================================================

/**
 * @brief NodeBrick::memorizeEdges
 */
void
postLearning(NetworkSegment &segment,
             Brick &brick)
{
    SynapseSection* sectionStart = getSynapseSectionBlock(segment.synapses);
    SynapseSection* sectionEnd = sectionStart + segment.synapses.numberOfItems;

    // iterate over all edge-sections
    for(SynapseSection* section = sectionStart;
        section < sectionEnd;
        section++)
    {
        if(section->status != ACTIVE_SECTION) {
            continue;
        }

        // update values based on the memorizing-value
        Synapse* end = section->synapses + section->numberOfSynapses;
        for(Synapse* synapse = section->synapses;
            synapse < end;
            synapse++)
        {

            // normal learn
            /*if(edge->inProcess == 1
                    && m_globalValue.globalLearningTemp != 0.0f
                    && edge->targetNodeId != NUMBER_OF_NODES_PER_BRICK - 1)
            {
                const float diff = 1.0f - edge->memorize;
                edge->weight += (diff * m_globalValue.globalLearningTemp);
            }*/

            // mem
            if(brick.globalValues.globalMemorizingTemp != 0.0f)
            {
                synapse->memorize = brick.globalValues.globalMemorizingTemp;
                if(synapse->memorize > 1.0f) {
                    synapse->memorize = 1.0f;
                }
            }
        }
    }
}

//==================================================================================================

/**
 * @brief memorizeEdges
 * @param brick
 */
void
memorizeSynapses(NetworkSegment &segment,
                 Brick &brick)
{
    SynapseSection* sectionStart = getSynapseSectionBlock(segment.synapses);
    SynapseSection* sectionEnd = sectionStart + segment.synapses.numberOfItems;

    // iterate over all synapse-sections
    uint32_t sectionPos = 0;
    for(SynapseSection* synapseSection = sectionStart;
        synapseSection < sectionEnd;
        synapseSection++)
    {
        // skip if section is deleted
        if(synapseSection->status != ACTIVE_SECTION)
        {
            sectionPos++;
            continue;
        }

        // update values based on the memorizing-value
        Synapse* end = synapseSection->synapses + synapseSection->numberOfSynapses;
        for(Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            if(synapse->inProcess == 1) {
                continue;
            }

            const float newWeight = synapse->weight * (1.0f - synapse->memorize);
            synapse->weight -= newWeight;
            synapseSection->totalWeight -= newWeight;
        }

        // delete dynamic item if value is too low
        EdgeSection* edgeSection = &getEdgeBlock(brick.edges)[synapseSection->sourceId];
        assert(edgeSection->status == ACTIVE_SECTION);

        if(synapseSection->totalWeight < DELETE_SYNAPSE_BORDER)
        {
            deleteDynamicItem(segment.synapses, sectionPos);
            processUpdateDeleteEdge(brick, *edgeSection, synapseSection->sourceId, 22);
        }
        else
        {
            const float updateValue = synapseSection->totalWeight;
            processUpdateSetEdge(brick, *edgeSection, updateValue, 22);
        }

        sectionPos++;
    }
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

    // write value to the internal ring-buffer
    Node* node = &getNodeBlock(segment.nodes)[brick.nodePos];
    brick.outBuffer[brick.outBufferPos] += node->currentState;
    brick.outBufferPos = (brick.outBufferPos + 1) % 10;
    node->currentState /= NODE_COOLDOWN;

    // summarize the ring-buffer and get the average value
    float result = 0.0f;
    for(uint32_t i = 0; i < 10; i++)
    {
        result += brick.outBuffer[i];
    }
    result /= 10.0f;

    return result;
}

//==================================================================================================

/**
 * @brief reportStatus
 */
void
writeMonitoringOutput(Brick &brick,
                      DataBuffer &buffer)
{
    GlobalValues globalValue = RootObject::m_globalValuesHandler->getGlobalValues();

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
