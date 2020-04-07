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

    if(RootObject::m_clientSession != nullptr) {
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
    sendNeighborBuffer(*sourceNeighbor);
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
 * @brief processOutputNodes
 * @param brick
 * @return
 */
uint16_t
processOutputNodes(Brick &brick)
{
    const DataConnection data = brick.dataConnections[NODE_DATA];
    // process nodes
    Node* start = static_cast<Node*>(data.buffer.data);
    Node* end = start + data.numberOfItems;

    // iterate over all nodes in the brick
    for(Node* node = start;
        node < end;
        node++)
    {
        node->currentState /= NODE_COOLDOWN;
    }
    return 0;
}

//==================================================================================================

/**
 * processing of the nodes of a specific node-brick
 *
 * @return number of active nodes in this brick
 */
uint16_t
processNodes(Brick &brick, float* weightMap)
{
    DataConnection* data = &brick.dataConnections[NODE_DATA];

    if(brick.isOutputBrick == 1)
    {
        // process nodes
        Node* start = static_cast<Node*>(data->buffer.data);
        Node* end = start + data->numberOfItems;

        // iterate over all nodes in the cluster
        for(Node* node = start;
            node < end;
            node++)
        {
            node->currentState /= NODE_COOLDOWN;
        }
        return 0;
    }

    uint16_t numberOfActiveNodes = 0;
    float totalPotential = 0.0f;

    // process nodes
    Node* start = (Node*)data->buffer.data;
    Node* end = start + data->numberOfItems;

    // iterate over all nodes in the brick
    for(Node* node = start;
        node < end;
        node++)
    {
        // limit the node-state to 255
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
            processAxon(brick, tempNode.targetAxonId, path, weight, weightMap);
        }
        // post-steps
        if(node->refractionTime != 0) {
            node->refractionTime--;
        }
        if(node->currentState < 0.0f) {
            node->currentState = 0.0f;
        }

        if(node->currentState > 1.2f * node->border) {
            node->tooHeight = 1;
        } else {
            node->tooHeight = 0;
        }

        // make cooldown in the node
        totalPotential += node->potential;
        node->potential /= NODE_COOLDOWN;
        node->currentState /= NODE_COOLDOWN;
    }

    //m_monitoringProcessingData.numberOfActiveAxons += numberOfActiveNodes;
    //m_monitoringProcessingData.averagetAxonPotential += totalPotential;

    //m_neighborInfo.localLearing += (float) numberOfActiveNodes / (float)m_brickData.numberOfStaticItems;
    return numberOfActiveNodes;
}

//==================================================================================================

/**
 * @brief NodeBrick::memorizeEdges
 */
void
postLearning(Brick &brick)
{
    const DataConnection* data = &brick.dataConnections[SYNAPSE_DATA];
    if(data->inUse != 1) {
        return;
    }

    SynapseSection* sectionStart = getSynapseSectionBlock(data);
    SynapseSection* sectionEnd = sectionStart + data->numberOfItems;

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
memorizeSynapses(Brick &brick)
{
    const DataConnection* data = &brick.dataConnections[SYNAPSE_DATA];
    if(data->inUse != 1) {
        return;
    }

    SynapseSection* sectionStart = getSynapseSectionBlock(data);
    SynapseSection* sectionEnd = sectionStart + data->numberOfItems;

    // iterate over all synapse-sections
    uint32_t sectionPos = 0;
    for(SynapseSection* section = sectionStart;
        section < sectionEnd;
        section++)
    {
        // skip if section is deleted
        if(section->status != ACTIVE_SECTION)
        {
            sectionPos++;
            continue;
        }

        // update values based on the memorizing-value
        Synapse* end = section->synapses + section->numberOfSynapses;
        for(Synapse* synapse = section->synapses;
            synapse < end;
            synapse++)
        {
            if(synapse->inProcess == 1) {
                continue;
            }

            const float newWeight = synapse->weight * (1.0f - synapse->memorize);
            synapse->weight -= newWeight;
        }

        // delete dynamic item if value is too low
        const DataConnection* connection = &brick.dataConnections[EDGE_DATA];
        if(section->totalWeight < DELETE_SYNAPSE_BORDER)
        {
            EdgeSection* currentSection = &getEdgeBlock(connection)[section->sourceId];
            processUpdateDeleteEdge(brick, *currentSection, section->sourceId, 22);
            deleteDynamicItem(brick, SYNAPSE_DATA, sectionPos);
        }
        else
        {
            EdgeSection* currentSection = &getEdgeBlock(connection)[section->sourceId];
            const float updateValue = section->totalWeight;
            if(updateValue > 0.0f) {
                processUpdateSetEdge(brick, *currentSection, updateValue, 22);
            }
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
getSummedValue(Brick &brick)
{
    // precheck
    if(brick.isOutputBrick > 0) {
        return false;
    }

    // get and check connection-item
    DataConnection* data = &brick.dataConnections[NODE_DATA];
    if(data->buffer.data == nullptr) {
        return 0.0f;
    }

    // iterate over all nodes in the brick and
    // summarize the states of all nodes
    Node* start = static_cast<Node*>(data->buffer.data);
    Node* end = start + data->numberOfItems;
    float sum = 0.0f;
    for(Node* node = start;
        node < end;
        node++)
    {
        sum += node->currentState;
    }

    // write value to the internal ring-buffer
    brick.outBuffer[brick.outBufferPos] = sum;
    brick.outBufferPos = (brick.outBufferPos + 1) % 10;

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
    const DataConnection* edgeConnection = &brick.dataConnections[EDGE_DATA];
    if(edgeConnection->inUse == 1) {
        monitoringMessage.numberOfEdgeSections = edgeConnection->numberOfItems
                                                 - edgeConnection->numberOfDeletedDynamicItems;
    }

    // nodes
    const DataConnection* nodeConnection = &brick.dataConnections[NODE_DATA];
    if(nodeConnection->inUse == 1) {
        monitoringMessage.numberOfNodes = nodeConnection->numberOfItems
                                          - nodeConnection->numberOfDeletedDynamicItems;
    }

    // synapses
    const DataConnection* synapseConnection = &brick.dataConnections[SYNAPSE_DATA];
    if(synapseConnection->inUse == 1) {
        monitoringMessage.numberOfSynapseSections = synapseConnection->numberOfItems
                                                   - synapseConnection->numberOfDeletedDynamicItems;
    }

    monitoringMessage.globalLearning = globalValue.globalLearningOffset;
    monitoringMessage.globalMemorizing = globalValue.globalMemorizingOffset;

    Kitsunemimi::addObjectToBuffer(buffer, &monitoringMessage);
}

//==================================================================================================

/**
 * @brief writeOutput
 * @param brick
 * @param buffer
 */
void
writeClientOutput(Brick &brick,
                  DataBuffer &buffer)
{
    Kitsunemimi::Kyouko::MindOutputData outputMessage;
    outputMessage.value = getSummedValue(brick);
    outputMessage.brickId = brick.brickId;

    Kitsunemimi::addObjectToBuffer(buffer, &outputMessage);
}

//==================================================================================================

} // namespace KyoukoMind
