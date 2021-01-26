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

inline float
createSynapse(SynapseSection* synapseSection,
              Synapse* synapse,
              float weight,
              const uint nodeBrickId)
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    // get random node-id as target
    uint32_t* randValues = getBuffer<uint32_t>(KyoukoRoot::m_segment->randomIntValues);
    synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
    const uint targetNodeIdInBrick = randValues[synapseSection->randomPos] % globalValue->numberOfNodesPerBrick;

    // set initial values for the new synapse
    const uint32_t nodeOffset = nodeBrickId * globalValue->numberOfNodesPerBrick;
    synapse->targetNodeId = static_cast<uint16_t>(targetNodeIdInBrick + nodeOffset);
    synapse->hardening = 0.0f;

    // make to blocking synapse by random
    synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
    if(randValues[synapseSection->randomPos] % 2 == 0) {
        synapse->sign = -1;
    }

    synapse->dynamicWeight += weight;

    if(synapseSection->isOutput != 0)
    {
        if(globalValue->outputIndex < 0.0f) {
            synapse->sign = -1;
        } else {
            synapse->sign = 1;
        }
    }

    return synapse->dynamicWeight;
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
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    for(uint64_t i = 0; i < numberOfSynapseTransfers; i++)
    {
        const uint32_t synapseSectionId = synapseTransfers[i].synapseSectionId;
        if(synapseSectionId == UNINIT_STATE_32) {
            continue;
        }

        // check if section is new and schould be created
        if(synapseTransfers[i].isNew == 1)
        {
            SynapseSection newSection;
            newSection.status = ACTIVE_SECTION;
            newSection.randomPos = rand() % 1024;
            newSection.sourcePositionInSection = synapseTransfers[i].positionInEdge;
            newSection.sourceEdgeId = synapseTransfers[i].sourceEdgeId;
            newSection.sourceBrickId = synapseTransfers[i].brickId;
            newSection.totalWeight = 0.0f;
            newSection.isOutput = synapseTransfers[i].isOutput;

            synapseSections[synapseSectionId]= newSection;
        }

        SynapseSection* synapseSection = &synapseSections[synapseSectionId];
        synapseSection->isActive = 1;

        // run lerning-process by creating and updating synapses
        float toLearn = synapseTransfers[i].weight - synapseSection->totalWeight;
        toLearn = (toLearn < 0.0f) * 0.0f + (toLearn >= 0.0f) * toLearn;

        // iterate over all synapses in the section and update the target-nodes
        float weight = synapseTransfers[i].weight;
        Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;
        for(Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            if(weight <= 1.0f) {
                break;
            }

            const float random = (rand() % 1024) / 1024.0f;
            float usedLearn = toLearn * random;
            if(toLearn < 5.0f) {
                usedLearn = toLearn;
            }
            if(usedLearn > 1.0f)
            {
                std::cout<<"learn synaspse: "<<usedLearn<<std::endl;
                // createe new, if nessecary
                if(synapse->targetNodeId == UNINIT_STATE_16)
                {
                    const uint32_t nodeBrickId = synapseTransfers[i].nodeBrickId;
                    createSynapse(synapseSection, synapse, usedLearn, nodeBrickId);
                    toLearn -= usedLearn;
                }
                else
                {
                    // share learning-weight
                    const float diff = usedLearn * (1.0f - synapse->hardening);
                    synapse->dynamicWeight += diff;
                    toLearn -= diff;
                }

            }

            /*if(nodes[synapse->targetNodeId].border < 0.0f) {
                synapse->dynamicWeight *= fabs(globalValue->outputIndex);
            }*/

            // 1 because only one thread at the moment
            const ulong nodeBufferPosition = (1 * (numberOfNodes / 256)) + synapse->targetNodeId;
            const float synapseWeight = synapse->staticWeight + synapse->dynamicWeight;
            const float shareWeight = (weight > synapseWeight) * synapseWeight
                                      + (weight <= synapseWeight) * weight;

            nodes[nodeBufferPosition].currentState += shareWeight * static_cast<float>(synapse->sign);
            weight -= shareWeight;
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
        // TODO: when port to gpu: change 2 to 256 again
        for(uint pos = 1; pos < 2; pos++)
        {
            const ulong nodeBufferPosition = (pos * (numberOfNodes / 256)) + i;
            nodes[i].currentState += nodes[nodeBufferPosition].currentState;
            nodes[nodeBufferPosition].currentState = 0.0f;
        }
    }

    for(uint64_t i = 0; i < numberOfNodes / 256; i++)
    {
        Node* node = &nodes[i];
        if(node->border > 0.0f)
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

/**
 * @brief harden
 * @param value
 */
inline void
hardenSynpaseSection(SynapseSection* section, const float value)
{
    float updateValue = value;

    for(uint16_t i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++)
    {
        // skip unused synapse in section
        if(section->synapses[i].targetNodeId == UNINIT_STATE_16
                || section->synapses[i].hardening == 1.0f)
        {
            continue;
        }

        Synapse* synapse = &section->synapses[i];

        if(updateValue <= 0.05f
                && synapse->dynamicWeight + synapse->staticWeight > 1.0f)
        {
            break;
        }

        const float newValue = synapse->hardening + updateValue;
        synapse->hardening = (newValue > 1.0f) * 1.0f + (newValue <= 1.0f) * newValue;

        // update static weight value
        const float diff = synapse->dynamicWeight * updateValue;
        synapse->dynamicWeight -= diff;
        synapse->staticWeight += diff;

        updateValue /= 2.0f;
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

        if(synapseSection->isActive == 0)
        {
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
                const float synapseWeight = synapse->dynamicWeight + synapse->staticWeight;
                if(synapseWeight < globalValue->deleteSynapseBorder)
                {
                    synapse->dynamicWeight = 0.0f;
                    synapse->staticWeight = 0.0f;
                    synapse->targetNodeId = UNINIT_STATE_16;
                    synapse->sign = 1;
                }
                else
                {
                    synapseSection->totalWeight += synapseWeight;
                    hardening += synapse->hardening;
                }
            }
        }
        else
        {
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

                // set new synapse hardening value
                synapse->hardening += globalValue->lerningValue;
                synapse->hardening = (synapse->hardening > 1.0f) * 1.0f
                                     + (synapse->hardening <= 1.0f) * synapse->hardening;

                // update static weight value
                const float diff = synapse->dynamicWeight * globalValue->lerningValue;
                synapse->dynamicWeight -= diff;
                synapse->staticWeight += diff;

                //hardenSynpaseSection(synapseSection, globalValue->lerningValue);

                const float synapseWeight = synapse->dynamicWeight + synapse->staticWeight;
                synapseSection->totalWeight += synapseWeight;
                hardening += synapse->hardening;
            }
        }

        synapseSection->isActive = 0;

        squash(synapseSection);

        // create update-container for the host
        transferContainer.newWeight = synapseSection->totalWeight;
        transferContainer.targetId = synapseSection->sourceEdgeId;
        transferContainer.positionInEdge = synapseSection->sourcePositionInSection;
        transferContainer.deleteEdge = transferContainer.newWeight <= globalValue->deleteSynapseBorder;
        transferContainer.hardening = hardening / static_cast<float>(SYNAPSES_PER_SYNAPSESECTION);

        updateTransfers[i] = transferContainer;
    }
}

#endif // SYNAPSE_PROCESSING_H
