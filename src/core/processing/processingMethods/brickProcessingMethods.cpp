#include "brickProcessingMethods.h"

#include <core/bricks/brickObjects/brick.h>
#include <core/bricks/brickMethods/commonBrickMethods.h>

#include <core/processing/processingMethods/messageProcessingMethods.h>

#include <communicationStructs/monitorinContianer.h>
#include <communicationStructs/mindContainer.h>
#include <communicationStructs/commonMessages.h>

using Kitsune::Chan::Communication::MonitoringMessage;
using Kitsune::Chan::Communication::MindOutputData;
using Kitsune::Chan::Communication::TransferDataMessage;

namespace KyoukoMind
{

/**
 * @brief processOutputNodes
 * @param brick
 * @return
 */
uint16_t
processOutputNodes(Brick* brick)
{
    const DataConnection data = brick->dataConnections[NODE_DATA];
    // process nodes
    Node* start = (Node*)data.buffer.data;
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
processNodes(Brick* brick, float* weightMap)
{
    DataConnection* data = &brick->dataConnections[NODE_DATA];

    if(brick->isOutputBrick == 1)
    {
        // process nodes
        Node* start = (Node*)data->buffer.data;
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
postLearning(Brick* brick)
{
    const DataConnection* data = &brick->dataConnections[SYNAPSE_DATA];
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
            if(brick->globalValues.globalMemorizingTemp != 0.0f)
            {
                synapse->memorize = brick->globalValues.globalMemorizingTemp;
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
memorizeSynapses(Brick* brick)
{
    const DataConnection* data = &brick->dataConnections[SYNAPSE_DATA];
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
        section->makeClean();

        // delete dynamic item if value is too low
        if(section->getTotalWeight() < DELETE_SYNAPSE_BORDER)
        {
            EdgeSection* currentSection = &getEdgeBlock(&brick->dataConnections[EDGE_DATA])[section->sourceId];
            processUpdateDeleteEdge(brick, currentSection, section->sourceId, 24);
            deleteDynamicItem(brick, SYNAPSE_DATA, sectionPos);
        }
        else
        {
            EdgeSection* currentSection = &getEdgeBlock(&brick->dataConnections[EDGE_DATA])[section->sourceId];
            const float updateValue = section->getTotalWeight();
            if(updateValue > 0.0f) {
                processUpdateSetEdge(brick, currentSection, updateValue, 24);
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
bool
finishSide(Brick* brick, const uint8_t side)
{
    // precheck
    if(brick == nullptr
            || brick->neighbors[side].inUse == 0
            || side >= 25)
    {
        return false;
    }

    // reset incoming buffer
    brick->neighbors[side].incomBuffer.reset();

    // finish message and give it to the target-brick
    DataMessage* message = brick->neighbors[side].outgoBuffer.message;
    message->isLast = 1;
    assert(message->type != UNDEFINED_MESSAGE);
    Brick* targetBrick = KyoukoNetwork::m_brickHandler->getBrick(message->targetBrickId);
    targetBrick->neighbors[message->targetSide].incomBuffer.addMessage(message->currentPosition);

    // check if target is finish
    if(targetBrick->isReady()) {
        KyoukoNetwork::m_brickHandler->addToQueue(targetBrick);
    }

    // reinit outgoing buffer for next cycle
    OutgoingBuffer* outBuffer = &brick->neighbors[side].outgoBuffer;
    outBuffer->message = KyoukoNetwork::m_internalMessageBuffer->reserveBuffer();
    outBuffer->initMessage();
    assert(outBuffer->message->type != UNDEFINED_MESSAGE);

    return true;
}

/**
 * @brief finishCycle
 * @param brick
 * @param monitoringMessage
 * @param clientMessage
 */
void
finishCycle(Brick* brick,
            TransferDataMessage* monitoringMessage,
            TransferDataMessage* clientMessage)
{
    // finish standard-neighbors
    for(uint8_t side = 0; side < 25; side++)
    {
        finishSide(brick, side);
    }

    // finish monitoring
    if(monitoringMessage != nullptr)
    {
        assert(monitoringMessage->type != UNDEFINED_MESSAGE);

        monitoringMessage->source = MIND;
        monitoringMessage->target = MONITORING;
        KyoukoNetwork::m_mindClient->sendData((uint8_t*)monitoringMessage,
                                              sizeof(DataMessage));
        monitoringMessage->init();
    }

    // finish client
    if(clientMessage != nullptr
            && brick->isOutputBrick)
    {
        assert(clientMessage->type != UNDEFINED_MESSAGE);

        clientMessage->source = MIND;
        clientMessage->target = CLIENT;
        KyoukoNetwork::m_mindClient->sendData((uint8_t*)clientMessage,
                                              sizeof(DataMessage));
        clientMessage->init();
    }
}

/**
 * @brief reportStatus
 */
void
writeStatus(Brick* brick, TransferDataMessage* message)
{
    if(message == nullptr) {
        return;
    }

    GlobalValues globalValue = KyoukoNetwork::m_globalValuesHandler->getGlobalValues();

    // fill message
    MonitoringMessage monitoringMessage;
    monitoringMessage.brickId = brick->brickId;
    monitoringMessage.xPos = brick->brickPos.x;
    monitoringMessage.yPos = brick->brickPos.y;
    if(brick->dataConnections[EDGE_DATA].inUse == 1) {
        monitoringMessage.numberOfEdgeSections = brick->dataConnections[EDGE_DATA].numberOfItems
                - brick->dataConnections[EDGE_DATA].numberOfDeletedDynamicItems;
    }
    if(brick->dataConnections[NODE_DATA].inUse == 1) {
        monitoringMessage.numberOfNodes = brick->dataConnections[NODE_DATA].numberOfItems
                - brick->dataConnections[NODE_DATA].numberOfDeletedDynamicItems;
    }
    if(brick->dataConnections[SYNAPSE_DATA].inUse == 1) {
        monitoringMessage.numberOfSynapseSections = brick->dataConnections[SYNAPSE_DATA].numberOfItems
                - brick->dataConnections[SYNAPSE_DATA].numberOfDeletedDynamicItems;
    }
    monitoringMessage.globalLearning = globalValue.globalLearningOffset;
    monitoringMessage.globalMemorizing = globalValue.globalMemorizingOffset;

    // add new container to message
    assert(message->type != UNDEFINED_MESSAGE);
    memcpy(&message->data[message->size], &monitoringMessage,
           sizeof(MonitoringMessage));
    message->size += sizeof(MonitoringMessage);

    // send message if necessary
    if(message->size > 460)
    {
        message->source = MIND;
        message->target = MONITORING;
        KyoukoNetwork::m_mindClient->sendData((uint8_t*)message,
                                              sizeof(DataMessage));
        message->init();
    }
}

/**
 * @brief reportStatus
 */
void
writeOutput(Brick* brick, TransferDataMessage* message)
{
    if(message == nullptr) {
        return;
    }

    // fill message
    MindOutputData outputMessage;
    outputMessage.value = getSummedValue(brick);
    outputMessage.brickId = brick->brickId;

    assert(message->type != UNDEFINED_MESSAGE);

    memcpy(&message->data[message->size],
           &outputMessage,
           sizeof(MindOutputData));
    message->size += sizeof(MindOutputData);

    if(message->size > 460)
    {
        message->source = MIND;
        message->target = CLIENT;
        KyoukoNetwork::m_mindClient->sendData((uint8_t*)message,
                                              sizeof(DataMessage));
        message->init();
    }
}

}
