/**
 * @file        synapse_node_processing.cl
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

// const predefined values
#define UNINIT_STATE_64 0xFFFFFFFFFFFFFFFF
#define UNINIT_STATE_32 0xFFFFFFFF
#define UNINIT_STATE_24 0xFFFFFF
#define UNINIT_STATE_16 0xFFFF
#define UNINIT_STATE_8  0xFF

// common information
#define SYNAPSES_PER_SYNAPSESECTION 15

//==================================================================================================

enum SectionStatus
{
    UNDEFINED_SECTION = 0,
    ACTIVE_SECTION = 1,
    DELETED_SECTION = 2,
};

enum SynapseType
{
    UNDEFINED_SYNAPSE_TYPE = 0,
    POSITIVE_TYPE = 1,
    NEGATIVE_TYPE = 2,
};

//==================================================================================================

typedef struct SynapseTransfer_struct
{
    uint brickId;
    uint nodeBrickId;
    uint synapseSectionId;
    uint sourceEdgeId;
    ushort positionInEdge;
    uchar isNew;
    uchar padding[1];
    float weight;
} 
SynapseTransfer;

//==================================================================================================

typedef struct AxonTransfer_struct
{
    float weight;
    uint brickId;
} 
AxonTransfer;

//==================================================================================================

typedef struct UpdateTransfer_struct
{
    uint targetId;
    uchar positionInEdge;
    uchar deleteEdge;
    uchar padding[2];
    float newWeight;
    float hardening;
} 
UpdateTransfer;

//==================================================================================================

typedef struct Node_struct
{
    float currentState;
    float border;
    float potential;

    uchar refractionTime;
    uchar active;

    uchar padding[2];

    // Axon
    uint brickId;
    uint targetBrickDistance;
} 
Node;

//==================================================================================================

typedef struct Synapse_struct
{
    float staticWeight;
    float dynamicWeight;
    float harden;
    ushort targetNodeId;
    uchar inProcess;
    uchar type;
} 
Synapse;

//==================================================================================================

typedef struct SynapseSection_struct
{
    uchar status;
    uchar positionInEdge;

    ushort randomPos;

    uint sourceEdgeId;
    uint sourceBrickId;

    float totalWeight;

    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];
} 
SynapseSection;

//==================================================================================================

typedef struct GlobalValues_struct
{
    uint numberOfNodesPerBrick;

    float sensitivity;
    float lerningValue;
    float gliaValue;
    float initialMemorizing;
    float memorizingOffset;

    float nodeCooldown;
    float actionPotential;
    ulong refractionTime;

    float newSynapseBorder;
    float deleteSynapseBorder;

    float outputIndex;

    uchar padding[204];
}
GlobalValues;

//==================================================================================================

__kernel void
hardening(__global SynapseSection* synapseSections,
          const ulong numberOfSynapseSections,
          __global GlobalValues* globalValue,
          const ulong numberGlobalValue,
          __local uchar* localMemory,
          const ulong localMemorySize)
{
    // prepare coordinates
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);

    // prepare shared memory
    __local GlobalValues* localGlobalValue = (__local GlobalValues*)localMemory;
    localGlobalValue[0] = globalValue[0];
    __local SynapseSection* tempSectionMem = (__local SynapseSection*)&localMemory[256];

    if(localGlobalValue->lerningValue == 0.0f) {
        return;
    }

    for(uint i = globalId_x; i < numberOfSynapseSections; i = i + globalSize_x)
    {
        // skip unused section
        if(synapseSections[i].status == DELETED_SECTION) {
            continue;
        }

        // load data into shared memory
        tempSectionMem[localId_x] = synapseSections[i];
        __local SynapseSection* synapseSection = &tempSectionMem[localId_x];
        __local Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;

        // iterate over all synapses in synapse-section
        for(__local Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            // skip unused synapse in section
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                continue;
            }

            // set new synapse hardening value
            synapse->harden += localGlobalValue->lerningValue;
            synapse->harden = (synapse->harden > 1.0f) * 1.0f + (synapse->harden <= 1.0f) * synapse->harden;

            // update values
            const float diff = synapse->dynamicWeight * synapse->harden;
            synapse->dynamicWeight -= diff;
            synapse->staticWeight += diff;
        }

        // write changes back from shared memory to global memory
        synapseSections[i] = tempSectionMem[localId_x];
    }
}

//==================================================================================================

inline void
createSynapse(__local SynapseSection* synapseSection,
              __local Synapse* synapse,
              __global uint* randomInts,
              __local GlobalValues* globalValue,
              const uint nodeBrickId)
{
    // create new synapse if necessary
    if(synapse->targetNodeId == UNINIT_STATE_16) 
    {
        // get random node-id as target
        synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
        const uint targetNodeIdInBrick = randomInts[synapseSection->randomPos] % globalValue->numberOfNodesPerBrick;

        // set initial values for the new synapse
        synapse->targetNodeId = (ushort)(targetNodeIdInBrick + (nodeBrickId * globalValue->numberOfNodesPerBrick));
        synapse->harden = 0.0f;
    }
}

//--------------------------------------------------------------------------------------------------

inline float
updateSynapseWeight(__local SynapseSection* synapseSection,
                    __local Synapse* chosenSynapse,
                    const float weight,
                    __global uint* randomInts,
                    __global Node* nodes,
                    __local GlobalValues* globalValue,
                    const uint nodeBrickId)
{
    const Node tempNode = nodes[nodeBrickId * globalValue->numberOfNodesPerBrick];
    float usedWeight = 0.0f;
    
    if(tempNode.border != -1.0f)
    {
        usedWeight = weight * (1.0f - chosenSynapse->harden);

        // set type if necessary
        if(chosenSynapse->type == UNDEFINED_SYNAPSE_TYPE) 
        {
            synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
            const uint positiveValue = randomInts[synapseSection->randomPos] % 2;
            if(positiveValue == 0) {
                chosenSynapse->type = POSITIVE_TYPE;
            } else {
                chosenSynapse->type = NEGATIVE_TYPE;
            }
        }

        // set new weight
        if(chosenSynapse->type == NEGATIVE_TYPE) {
            chosenSynapse->dynamicWeight += -1.0f * usedWeight * globalValue->sensitivity;
        } else {
            chosenSynapse->dynamicWeight += usedWeight * globalValue->sensitivity;
        }

        synapseSection->totalWeight += usedWeight * globalValue->sensitivity;
    }
    else
    {
        usedWeight = weight * (1.0f - chosenSynapse->harden);
        chosenSynapse->dynamicWeight += usedWeight * globalValue->outputIndex;
        synapseSection->totalWeight += usedWeight * globalValue->outputIndex;
    }

    return usedWeight;
}

//--------------------------------------------------------------------------------------------------

inline void
rewightSynapse(__local SynapseSection* synapseSection,
               float weight,
               __global uint* randomInts,
               __global Node* nodes,
               __local GlobalValues* globalValue,
               const uint nodeBrickId)
{
    uint numberOfActiveSynapses = 0;
    __local Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;

    // iterate over all synapses in synapse-section
    for(__local Synapse* synapse = synapseSection->synapses;
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
            createSynapse(synapseSection, 
                          synapse, 
                          randomInts, 
                          globalValue, 
                          nodeBrickId);
        }
        
        weight -= updateSynapseWeight(synapseSection, 
                                      synapse, 
                                      weight, 
                                      randomInts, 
                                      nodes, 
                                      globalValue, 
                                      nodeBrickId);
    }
}

//--------------------------------------------------------------------------------------------------

__kernel void 
synapse_processing(__global SynapseTransfer* synapseTransfers,
                   const ulong numberOfSynapseTransfers,
                   __global Node* nodes,
                   const ulong numberOfNodes,
                   __global SynapseSection* synapseSections,
                   const ulong numberOfSynapseSections,
                   __global uint* randomInts,
                   const ulong numberRandomInts,
                   __global GlobalValues* globalValue,
                   const ulong numberGlobalValue,
                   __local uchar* localMemory,
                   const ulong localMemorySize)
{
    // calculate global ids
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);
    const uint numberOfBricks = numberOfNodes / globalValue->numberOfNodesPerBrick;
    const uint brickId = globalId_x / localSize_x; 

    // prepare shared memory
    __local GlobalValues* localGlobalValue = (__local GlobalValues*)localMemory;
    localGlobalValue[0] = globalValue[0];
    __local SynapseSection* tempSections = (__local SynapseSection*)&localMemory[256];

    for(ulong i = localId_x; i < numberOfSynapseTransfers; i = i + localSize_x)
    {
        // check if gpu-thread is to determine to handle this tranfer-container
        if(synapseTransfers[i].brickId != brickId) {
            continue;
        }

        // load target-section into shared memory
        const uint synapseSectionId = synapseTransfers[i].synapseSectionId;
        tempSections[localId_x] = synapseSections[synapseSectionId];
        __local SynapseSection* synapseSection = &tempSections[localId_x];

        // check if section is new and schould be created
        if(synapseTransfers[i].isNew == 1)
        {
            SynapseSection newSection;
            newSection.status = ACTIVE_SECTION;
            newSection.randomPos = (globalId_x * localId_x) % 1024;
            newSection.positionInEdge = synapseTransfers[i].positionInEdge;
            newSection.sourceEdgeId = synapseTransfers[i].sourceEdgeId;
            newSection.sourceBrickId = synapseTransfers[i].brickId;
            newSection.totalWeight = 0.0f;

            tempSections[localId_x] = newSection;

            // write result back
            synapseSections[synapseSectionId] = tempSections[localId_x];
        }

        // run lerning-process by creating and updating synapses
        const float weightDiff = synapseTransfers[i].weight - synapseSection->totalWeight;
        if(weightDiff > 0.0f)
        {
            const uint nodeBrickId = synapseTransfers[i].nodeBrickId;
            rewightSynapse(synapseSection, 
                           weightDiff, 
                           randomInts, 
                           nodes, 
                           localGlobalValue, 
                           nodeBrickId);
            synapseSections[synapseSectionId] = tempSections[localId_x];
        }

        // iterate over all synapses in the section and update the target-nodes
        float weight = synapseTransfers[i].weight;
        __local Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;
        for(__local Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            if(weight <= 0.001f) {
                break;
            }

            if(synapse->targetNodeId == UNINIT_STATE_16) {
                continue;
            }

            const ulong nodeBufferPosition = (get_local_id(0) * (numberOfNodes / 256)) + synapse->targetNodeId;
            const float synapseWeight = fabs(synapse->staticWeight) + fabs(synapse->dynamicWeight);
            if(weight > synapseWeight) 
            {
                nodes[nodeBufferPosition].currentState += synapse->staticWeight + synapse->dynamicWeight;
                weight -= synapseWeight;
            } 
            else 
            {
                nodes[nodeBufferPosition].currentState += weight;
                weight = 0.0f;
            }
        }
    }
}

//==================================================================================================

__kernel void 
sum_nodes(__global Node* nodes,
          const ulong numberOfNodes,
          __local uchar* localMemory,
          const ulong localMemorySize)
{
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);

    // prepare shared memory
    __local Node* tempNodes = (__local Node*)&localMemory[256];

    for(ulong i = globalId_x; i < numberOfNodes / 256; i = i + globalSize_x)
    {
        tempNodes[localId_x] = nodes[i];
        __local Node* node = &tempNodes[localId_x];

        for(uint pos = 1; pos < 256; pos++)
        {
            const ulong nodeBufferPosition = (pos * (numberOfNodes / 256)) + i;
            tempNodes[localId_x].currentState += nodes[nodeBufferPosition].currentState;
            nodes[nodeBufferPosition].currentState = 0.0f;
        }

        nodes[i] = tempNodes[localId_x];
    }
}

//==================================================================================================

__kernel void 
node_processing(__global AxonTransfer* axonTransfers,
                const ulong numberOfAxonTransfers,
                __global Node* nodes,
                const ulong numberOfNodes,
                __global GlobalValues* globalValue,
                const ulong numberGlobalValue,
                __local uchar* localMemory,
                const ulong localMemorySize)
{
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);

    // prepare shared memory
    __local GlobalValues* localGlobalValue = (__local GlobalValues*)localMemory;
    localGlobalValue[0] = globalValue[0];
    __local Node* tempNodes = (__local Node*)&localMemory[256];

    for(ulong i = globalId_x; i < numberOfNodes / 256; i = i + globalSize_x)
    {
        tempNodes[localId_x] = nodes[i];
        __local Node* node = &tempNodes[localId_x];

        if(node->border != -1.0f) 
        {
            // set to 255.0f, if value is too high
            const float cur = node->currentState;
            node->currentState = (cur > 255.0f) * 255.0f + (cur <= 255.0f) * cur;
            node->currentState = (cur < 0.000001f) * 0.0f + (cur >= 0.000001f) * cur;

            // check if active
            if(node->border < node->currentState
                    && node->refractionTime == 0)
            {
                node->potential = localGlobalValue->actionPotential;
                node->active = 1;
                node->refractionTime = localGlobalValue->refractionTime;
            }
            else if(node->refractionTime == 0)
            {
                node->active = 0;
            }

            // build new axon-transfer-edge, which is send back to the host
            AxonTransfer newEdge;
            newEdge.weight = node->active * node->potential * pow(localGlobalValue->gliaValue, node->targetBrickDistance);
            newEdge.brickId = node->brickId;
            axonTransfers[i] = newEdge;

            // post-steps
            node->refractionTime = node->refractionTime >> 1;

            // set to 0.0f, if value is negative
            const float newCur = node->currentState;
            node->currentState = (newCur < 0.0f) * 0.0f + (newCur >= 0.0f) * newCur;

            // make cooldown in the node
            node->potential /= localGlobalValue->nodeCooldown;
            node->currentState /= localGlobalValue->nodeCooldown;
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
            if(newEdge.weight != 0.0f) {
                printf("%f\n", newEdge.weight);
            }
            //node->currentState /= localGlobalValue->nodeCooldown;
        }

        // write changes back from shared memory to global memory
        nodes[i] = tempNodes[localId_x];
    }
}

//==================================================================================================

void 
squash(__local SynapseSection* synapseSections)
{
    Synapse tempBuffer[SYNAPSES_PER_SYNAPSESECTION];
    uint pos = 0;

    for(uint i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++)
    {
        tempBuffer[i].targetNodeId == UNINIT_STATE_16;

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

__kernel void
updating(__global UpdateTransfer* updateTransfers,
         const ulong numberOfUpdateTransfers,
         __global Node* nodes,
         const ulong numberOfNodes,
         __global SynapseSection* synapseSections,
         const ulong numberOfSynapseSections,
         __global GlobalValues* globalValue,
         const ulong numberGlobalValue,
         __local uchar* localMemory,
         const ulong localMemorySize)
{
    // prepare coordinates
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);

    // prepare shared memory
    __local GlobalValues* localGlobalValue = (__local GlobalValues*)localMemory;
    localGlobalValue[0] = globalValue[0];
    __local SynapseSection* tempSectionMem = (__local SynapseSection*)&localMemory[256];

    for(uint i = globalId_x; i < numberOfSynapseSections; i = i + globalSize_x)
    {
        // prepare new container
        UpdateTransfer transferContainer;
        transferContainer = transferContainer;
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
        tempSectionMem[localId_x] = synapseSections[i];
        __local SynapseSection* synapseSection = &tempSectionMem[localId_x];
        __local Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;

        synapseSection->totalWeight = 0.0f;
        float hardening = 0.0f;

        // iterate over all synapses in synapse-section
        for(__local Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            // skip unused synapse in section
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                continue;
            }
                              
            // update dynamic-weight-value of the synapse     
            if(nodes[synapse->targetNodeId].active == 0) {
                synapse->dynamicWeight = synapse->dynamicWeight * (globalValue->initialMemorizing + localGlobalValue->memorizingOffset);
            } else {
                synapse->dynamicWeight = synapse->dynamicWeight * 0.95f;
            }

            // check for deletion of the single synapse
            const float synapseWeight = fabs(synapse->dynamicWeight) + fabs(synapse->staticWeight);
            if(synapseWeight < localGlobalValue->deleteSynapseBorder) 
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
        transferContainer.deleteEdge = transferContainer.newWeight <= localGlobalValue->deleteSynapseBorder;
        transferContainer.hardening = hardening / (float)SYNAPSES_PER_SYNAPSESECTION;

        // mark section as deleted
        if(transferContainer.deleteEdge > 0) {
            synapseSection->status = DELETED_SECTION;
        }

        // write changes back from shared memory to global memory
        updateTransfers[i] = transferContainer;
        synapseSections[i] = tempSectionMem[localId_x];
    }
}
