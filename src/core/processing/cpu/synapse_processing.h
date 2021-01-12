#ifndef SYNAPSE_PROCESSING_H
#define SYNAPSE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/edges.h>
#include <core/objects/transfer_objects.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/item_buffer.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

//==================================================================================================

void
hardening()
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);

    if(globalValue->lerningValue == 0.0f) {
        return;
    }

    const uint64_t numberOfSynapseSections = KyoukoRoot::m_segment->synapses.itemCapacity;
    for(uint64_t i = 0; i < numberOfSynapseSections; i++)
    {
        // skip unused section
        if(synapseSections[i].status == DELETED_SECTION) {
            continue;
        }

        // load data into shared memory
        SynapseSection* synapseSection = &synapseSections[i];
        Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;

        // iterate over all synapses in synapse-section
        for(Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            // skip unused synapse in section
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                continue;
            }

            // set new synapse hardening value
            synapse->harden += globalValue->lerningValue;
            synapse->harden = (synapse->harden > 1.0f) * 1.0f
                              + (synapse->harden <= 1.0f) * synapse->harden;

            // update values
            const float diff = synapse->dynamicWeight * globalValue->lerningValue;
            synapse->dynamicWeight -= diff;
            synapse->staticWeight += diff;
        }
    }
}

//==================================================================================================

inline float
updateSynapseWeight(SynapseSection* synapseSection,
                    Synapse* chosenSynapse,
                    const float weight,
                    Node* nodes,
                    GlobalValues* globalValue,
                    const uint nodeBrickId)
{
    const Node tempNode = nodes[nodeBrickId * globalValue->numberOfNodesPerBrick];
    float usedWeight = 0.0f;

    if(tempNode.border <= -1.0f)
    {
        usedWeight = weight * (1.0f - chosenSynapse->harden) * globalValue->sensitivity;

        // set type if necessary
        if(chosenSynapse->type == UNDEFINED_SYNAPSE_TYPE)
        {
            uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);
            synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
            if(randValues[synapseSection->randomPos] % 2 == 0) {
                chosenSynapse->type = POSITIVE_TYPE;
            } else {
                chosenSynapse->type = NEGATIVE_TYPE;
            }
        }

        // set new weight
        if(chosenSynapse->type == NEGATIVE_TYPE) {
            chosenSynapse->dynamicWeight += -1.0f * usedWeight;
        } else {
            chosenSynapse->dynamicWeight += usedWeight;
        }
    }
    else
    {
        usedWeight = weight * (1.0f - chosenSynapse->harden) * globalValue->outputIndex;
        chosenSynapse->dynamicWeight += usedWeight;
    }

    return fabs(usedWeight);
}

//--------------------------------------------------------------------------------------------------

inline void
rewightSynapse(SynapseSection* synapseSection,
               float weight,
               Node* nodes,
               const uint nodeBrickId)
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    // iterate over all synapses in synapse-section
    Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;
    for(Synapse* synapse = synapseSection->synapses;
        synapse < end;
        synapse++)
    {
        if(globalValue->sensitivity == 0.0f
            && globalValue->outputIndex == 0.0f)
        {
            return;
        }

        if(weight < 1.0f) {
            return;
        }

        if(synapse->targetNodeId == UNINIT_STATE_16)
        {
            // get random node-id as target
            uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);
            synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
            const uint targetNodeIdInBrick = randValues[synapseSection->randomPos] % globalValue->numberOfNodesPerBrick;

            // set initial values for the new synapse
            synapse->targetNodeId = (ushort)(targetNodeIdInBrick + (nodeBrickId * globalValue->numberOfNodesPerBrick));
            synapse->harden = 0.0f;
        }

        weight -= updateSynapseWeight(synapseSection,
                                      synapse,
                                      weight,
                                      nodes,
                                      globalValue,
                                      nodeBrickId);
    }
}

//--------------------------------------------------------------------------------------------------

void
synapse_processing()
{
    const uint64_t numberOfNodes = KyoukoRoot::m_segment->nodes.numberOfItems;
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    SynapseTransfer* synapseTransfers = getBuffer<SynapseTransfer>(KyoukoRoot::m_segment->synapseTransfers);
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);
    const uint64_t numberOfSynapseTransfers = KyoukoRoot::m_segment->synapseTransfers.numberOfItems;

    for(uint64_t i = 0; i < numberOfSynapseTransfers; i++)
    {
        const uint32_t synapseSectionId = synapseTransfers[i].synapseSectionId;
        if(synapseSectionId == UNINIT_STATE_32) {
            continue;
        }

        SynapseSection* synapseSection = &synapseSections[synapseSectionId];

        // check if section is new and schould be created
        if(synapseTransfers[i].isNew == 1)
        {
            SynapseSection newSection;
            newSection.status = ACTIVE_SECTION;
            newSection.randomPos = rand() % 1024;
            newSection.positionInEdge = synapseTransfers[i].positionInEdge;
            newSection.sourceEdgeId = synapseTransfers[i].sourceEdgeId;
            newSection.sourceBrickId = synapseTransfers[i].brickId;
            newSection.totalWeight = 0.0f;

            synapseSections[synapseSectionId]= newSection;
        }

        // run lerning-process by creating and updating synapses
        const float weightDiff = synapseTransfers[i].weight - synapseSection->totalWeight;
        if(weightDiff > 0.0f)
        {
            const uint nodeBrickId = synapseTransfers[i].nodeBrickId;
            rewightSynapse(synapseSection,
                           weightDiff,
                           nodes,
                           nodeBrickId);
        }

        // iterate over all synapses in the section and update the target-nodes
        float weight = synapseTransfers[i].weight;
        Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;
        for(Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            if(weight <= 1.0f
                    || synapse->targetNodeId == UNINIT_STATE_16)
            {
                break;
            }

            // 1 because only one thread at the moment
            const ulong nodeBufferPosition = (1 * (numberOfNodes / 256)) + synapse->targetNodeId;
            const float synapseWeight = synapse->staticWeight + synapse->dynamicWeight;
            const float absSynapseWeight = fabs(synapseWeight);
            const float shareWeight = (weight > absSynapseWeight) * synapseWeight
                                      + (weight <= absSynapseWeight) * weight;

            nodes[nodeBufferPosition].currentState += shareWeight;
            weight -= shareWeight;
        }
    }
}

//==================================================================================================

void
sum_nodes()
{
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    const uint64_t numberOfNodes = KyoukoRoot::m_segment->nodes.numberOfItems;

    for(uint64_t i = 0; i < numberOfNodes / 256; i++)
    {
        // TODO: when port to gpu: change 2 to 256 again
        for(uint pos = 1; pos < 2; pos++)
        {
            const ulong nodeBufferPosition = (pos * (numberOfNodes / 256)) + i;
            nodes[i].currentState += nodes[nodeBufferPosition].currentState;
            nodes[nodeBufferPosition].currentState = 0.0f;
        }
    }
}

//==================================================================================================

void
node_processing()
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    AxonTransfer* axonTransfers = getBuffer<AxonTransfer>(KyoukoRoot::m_segment->axonTransfers);
    const uint64_t numberOfNodes = KyoukoRoot::m_segment->nodes.numberOfItems;

    for(uint64_t i = 0; i < numberOfNodes / 256; i++)
    {
        Node* node = &nodes[i];
        if(node->border <= -1.0f)
        {
            // set to 255.0f, if value is too high
            const float cur = node->currentState;
            node->currentState = (cur > 255.0f) * 255.0f + (cur <= 255.0f) * cur;
            node->currentState = (cur < 0.000001f) * 0.0f + (cur >= 0.000001f) * cur;

            // check if active
            if(node->border < node->currentState
                    && node->refractionTime == 0)
            {
                node->potential = globalValue->actionPotential;
                node->active = 1;
                node->refractionTime = globalValue->refractionTime;
            }
            else if(node->refractionTime == 0)
            {
                node->active = 0;
            }

            // build new axon-transfer-edge, which is send back to the host
            AxonTransfer newEdge;
            float up = static_cast<float>(pow(globalValue->gliaValue, node->targetBrickDistance));
            newEdge.weight = node->active * node->potential * up;
            newEdge.brickId = node->brickId;
            axonTransfers[i] = newEdge;

            // post-steps
            node->refractionTime = node->refractionTime >> 1;

            // set to 0.0f, if value is negative
            const float newCur = node->currentState;
            node->currentState = (newCur < 0.0f) * 0.0f + (newCur >= 0.0f) * newCur;

            // make cooldown in the node
            node->potential /= globalValue->nodeCooldown;
            node->currentState /= globalValue->nodeCooldown;
        }
        else
        {
            node->active = 0;

            // build new axon-transfer-edge, which is send back to the host
            AxonTransfer newEdge;
            newEdge.weight = node->currentState;
            newEdge.brickId = node->brickId;
            axonTransfers[i] = newEdge;
            node->currentState = 0.0f;
        }
    }
}

//==================================================================================================

void
squash(SynapseSection* synapseSections)
{
    Synapse tempBuffer[SYNAPSES_PER_SYNAPSESECTION];
    uint pos = 0;

    for(uint i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++)
    {
        tempBuffer[i].targetNodeId = UNINIT_STATE_16;

        if(synapseSections->synapses[i].targetNodeId != UNINIT_STATE_16)
        {
            tempBuffer[pos] = synapseSections->synapses[i];
            pos++;
        }
    }

    for(uint i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++) {
        synapseSections->synapses[i] = tempBuffer[i];
    }
}

//--------------------------------------------------------------------------------------------------

void
updating()
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Node* nodes = getBuffer<Node>(KyoukoRoot::m_segment->nodes);
    UpdateTransfer* updateTransfers = getBuffer<UpdateTransfer>(KyoukoRoot::m_segment->updateTransfers);
    SynapseSection* synapseSections = getBuffer<SynapseSection>(KyoukoRoot::m_segment->synapses);
    const uint64_t numberOfSynapseSections = KyoukoRoot::m_segment->synapses.itemCapacity;

    for(uint64_t i = 0; i < numberOfSynapseSections; i++)
    {
        // prepare new container
        UpdateTransfer transferContainer;
        transferContainer.newWeight = 0.0f;
        transferContainer.targetId = UNINIT_STATE_32;
        transferContainer.positionInEdge = UNINIT_STATE_8;
        transferContainer.deleteEdge = 0;

        // skip if section is deleted
        if(synapseSections[i].status == DELETED_SECTION)
        {
            updateTransfers[i] = transferContainer;
            continue;
        }

        // load data into shared memory
        SynapseSection* synapseSection = &synapseSections[i];
        synapseSection->totalWeight = 0.0f;
        float hardening = 0.0f;

        // iterate over all synapses in synapse-section
        Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;
        for(Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            // skip unused synapse in section
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                continue;
            }

            // update dynamic-weight-value of the synapse
            if(nodes[synapse->targetNodeId].active == 0) {
                synapse->dynamicWeight = synapse->dynamicWeight * globalValue->initialMemorizing;
            } else {
                synapse->dynamicWeight = synapse->dynamicWeight * 0.95f;
            }

            // check for deletion of the single synapse
            const float synapseWeight = fabs(synapse->dynamicWeight) + fabs(synapse->staticWeight);
            if(synapseWeight < globalValue->deleteSynapseBorder)
            {
                synapse->dynamicWeight = 0.0f;
                synapse->staticWeight = 0.0f;
                synapse->targetNodeId = UNINIT_STATE_16;
                synapse->type = UNDEFINED_SYNAPSE_TYPE;
            }
            else
            {
                synapseSection->totalWeight += synapseWeight;
                hardening += synapse->harden;
            }
        }

        squash(synapseSection);

        // create update-container for the host
        transferContainer.newWeight = synapseSection->totalWeight;
        transferContainer.targetId = synapseSection->sourceEdgeId;
        transferContainer.positionInEdge = synapseSection->positionInEdge;
        transferContainer.deleteEdge = transferContainer.newWeight <= globalValue->deleteSynapseBorder;
        transferContainer.hardening = hardening / static_cast<float>(SYNAPSES_PER_SYNAPSESECTION);

        // mark section as deleted
        if(transferContainer.deleteEdge > 0) {
            synapseSection->status = DELETED_SECTION;
        }
    }
}

#endif // SYNAPSE_PROCESSING_H
