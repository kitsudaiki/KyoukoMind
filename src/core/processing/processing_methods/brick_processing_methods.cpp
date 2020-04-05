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

#include <core/processing/processing_methods/message_processing_methods.h>
#include <core/processing/processing_methods/brick_item_methods.h>

#include <libKitsunemimiKyoukoCommon/communication_structs/monitoring_contianer.h>
#include <libKitsunemimiKyoukoCommon/communication_structs/client_contianer.h>
#include <libKitsunemimiKyoukoCommon/communication_structs/mind_container.h>

namespace KyoukoMind
{

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
        makeClean(*section);

        // delete dynamic item if value is too low
        const DataConnection* connection = &brick.dataConnections[EDGE_DATA];
        if(getTotalWeight(*section) < DELETE_SYNAPSE_BORDER)
        {
            EdgeSection* currentSection = &getEdgeBlock(connection)[section->sourceId];
            processUpdateDeleteEdge(brick, *currentSection, section->sourceId, 24);
            deleteDynamicItem(brick, SYNAPSE_DATA, sectionPos);
        }
        else
        {
            EdgeSection* currentSection = &getEdgeBlock(connection)[section->sourceId];
            const float updateValue = getTotalWeight(*section);
            if(updateValue > 0.0f) {
                processUpdateSetEdge(brick, *currentSection, updateValue, 24);
            }
        }
        sectionPos++;
    }
}

/**
 * @brief finishSide
 * @param brick
 * @param side
 */
void
finishSide(Brick &brick,
           const uint8_t sourceSide)
{
    Neighbor* sourceNeighbor = &brick.neighbors[sourceSide];
    Brick* targetBrick = RootObject::m_brickHandler->getBrick(sourceNeighbor->targetBrickId);
    Neighbor* targetNeighbor = &targetBrick->neighbors[sourceNeighbor->targetSide];

    sendBuffer(*sourceNeighbor, *targetNeighbor);

    // check if target is finish
    updateReadyStatus(*targetBrick, sourceNeighbor->targetSide);
    if(isReady(*targetBrick)) {
        RootObject::m_brickHandler->addToQueue(targetBrick);
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

/**
 * @brief finishCycle
 * @param brick
 * @param monitoringMessage
 * @param clientMessage
 */
void
finishCycle(Brick &brick,
            DataBuffer &clientMessage,
            DataBuffer &monitoringMessage)
{
    // finish standard-neighbors
    for(uint8_t side = 0; side < 25; side++)
    {
        finishSide(brick, side);
    }

    clientMessage.bufferPosition = 0;
    monitoringMessage.bufferPosition = 0;

    // TODO: send
}

/**
 * @brief reportStatus
 */
void
writeStatus(Brick &brick,
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

/**
 * @brief writeOutput
 * @param brick
 * @param buffer
 */
void
writeOutput(Brick &brick,
            DataBuffer &buffer)
{
    Kitsunemimi::Kyouko::MindOutputData outputMessage;
    outputMessage.value = getSummedValue(brick);
    outputMessage.brickId = brick.brickId;

    Kitsunemimi::addObjectToBuffer(buffer, &outputMessage);
}

} // namespace KyoukoMind
