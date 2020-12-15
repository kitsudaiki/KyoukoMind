/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
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

//==================================================================================================

typedef struct SynapseTransfer_struct
{
    uint brickId;
    uint synapseSectionId;
    uint sourceEdgeId;
    uchar positionInEdge;
    uchar isNew;
    uchar padding[2];
    float weight;
} 
SynapseTransfer;

//==================================================================================================

typedef struct AxonTransfer_struct
{
    float weight;
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
    //float weight;
    float memorize;
    ushort targetNodeId;
    uchar inProcess;
    uchar somaDistance;
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
    float memorizingValue;
    float lerningValue;
    float gliaValue;

    float initialMemorizing;

    float nodeCooldown;
    float actionPotential;
    ulong refractionTime;

    float newSynapseBorder;
    float deleteSynapseBorder;
    uint maxSomaDistance;

    uint numberOfNodesPerBrick;

    uchar padding[208];
}
GlobalValues;

//==================================================================================================

inline void
singleLearningStep(__local SynapseSection* synapseSection,
                   const float weight,
                   __global uint* randomInts,
                   __local GlobalValues* globalValue,
                   const uint brickId)
{
    synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
    const uint choosePosition = randomInts[synapseSection->randomPos] % SYNAPSES_PER_SYNAPSESECTION;
    __local Synapse* chosenSynapse = &synapseSection->synapses[choosePosition];

    // create new synapse if necessary
    if(chosenSynapse->targetNodeId == UNINIT_STATE_16) 
    {
        synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
        const uint targetNodeIdInBrick = randomInts[synapseSection->randomPos] % globalValue->numberOfNodesPerBrick;

        chosenSynapse->targetNodeId = (ushort)(targetNodeIdInBrick + (brickId * globalValue->numberOfNodesPerBrick));
        chosenSynapse->memorize = 0.5f;
        chosenSynapse->staticWeight = 0.0f;
        chosenSynapse->dynamicWeight = 0.0f;
    }

    // calculate new value
    synapseSection->synapses[choosePosition].dynamicWeight += weight;
}

//--------------------------------------------------------------------------------------------------

__kernel void 
synapse_processing(__global const SynapseTransfer* synapseTransfers,
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

    __local GlobalValues* localGlobalValue = (__local GlobalValues*)localMemory;
    localGlobalValue[0] = globalValue[0];
    __local SynapseSection* tempSections = (__local SynapseSection*)&localMemory[256];

    for(ulong i = localId_x; i < numberOfSynapseTransfers; i = i + localSize_x)
    {
        if(synapseTransfers[i].brickId != brickId) {
            continue;
        }

        const uint synapseSectionId = synapseTransfers[i].synapseSectionId;
        tempSections[localId_x] = synapseSections[synapseSectionId];

        if(synapseTransfers[i].isNew == 1)
        {
            SynapseSection newSection;
            newSection.status = ACTIVE_SECTION;
            newSection.randomPos = (globalId_x * localId_x) % 1024;
            newSection.positionInEdge = synapseTransfers[i].positionInEdge;
            newSection.sourceEdgeId = synapseTransfers[i].sourceEdgeId;
            tempSections[localId_x] = newSection;
        }

        if(tempSections[localId_x].status != ACTIVE_SECTION
            || synapseTransfers[i].weight < 0.000001f) 
        {
            continue;
        }

        __local SynapseSection* synapseSection = &tempSections[localId_x];

        // run lerning-process by creating and updating synapses
        const float weightDiff = synapseTransfers[i].weight - synapseSection->totalWeight;
        if(weightDiff > 0.0f)
        {
            singleLearningStep(synapseSection, 0.5f * weightDiff, randomInts, localGlobalValue, brickId);
            singleLearningStep(synapseSection, 0.3f * weightDiff, randomInts, localGlobalValue, brickId);
            singleLearningStep(synapseSection, 0.2f * weightDiff, randomInts, localGlobalValue, brickId);
        }

        // limit ration to 1.0f
        float ratio = synapseTransfers[i].weight / synapseSection->totalWeight;
        ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

        __local Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;
        for(__local Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            nodes[synapse->targetNodeId].currentState += (synapse->staticWeight + synapse->dynamicWeight) * ratio;
        }

        // write result back
        synapseSections[synapseSectionId] = tempSections[localId_x];
        synapseSections[synapseSectionId].sourceBrickId = brickId;
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

    __local GlobalValues* localGlobalValue = (__local GlobalValues*)localMemory;
    localGlobalValue[0] = globalValue[0];
    __local Node* tempNodes = (__local Node*)&localMemory[256];

    for(ulong i = globalId_x; i < numberOfNodes; i = i + globalSize_x)
    {
        tempNodes[localId_x] = nodes[i];
        __local Node* node = &tempNodes[localId_x];

        // set to 255.0f, if value is too high
        const float cur = node->currentState;
        node->currentState = (cur > 255.0f) * 255.0f + (cur <= 255.0f) * cur;
        node->currentState = (cur < 0.000001f) * 0.0f + (cur >= 0.000001f) * cur;

        // check if active
        if(node->border <= node->currentState
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
        axonTransfers[i] = newEdge;

        // post-steps
        node->refractionTime = node->refractionTime >> 1;

        // set to 0.0f, if value is negative
        const float newCur = node->currentState;
        node->currentState = (newCur < 0.0f) * 0.0f + (newCur >= 0.0f) * newCur;

        // make cooldown in the node
        node->potential /= localGlobalValue->nodeCooldown;
        node->currentState /= localGlobalValue->nodeCooldown;

        nodes[i] = tempNodes[localId_x];
    }
}

//==================================================================================================

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
            transferContainer.newWeight = 0.0f;
            transferContainer.targetId = UNINIT_STATE_32;
            transferContainer.positionInEdge = UNINIT_STATE_8;
            transferContainer.deleteEdge = 0;
            updateTransfers[i] = transferContainer;

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
            // update synapse weight
            const int active = nodes[synapse->targetNodeId].active != 0;
            const float diff = synapse->dynamicWeight * (float)active * localGlobalValue->lerningValue;
            synapse->dynamicWeight -= diff;
            synapse->staticWeight += diff;

            // update dynamicWeight
            synapse->dynamicWeight = synapse->dynamicWeight * synapse->memorize;

            const float synapseWeight = fabs(synapse->dynamicWeight + synapse->staticWeight);

            // check for deletion of the single synapse
            if(synapseWeight < localGlobalValue->deleteSynapseBorder) 
            {
                synapse->dynamicWeight = 0.0f;
                synapse->staticWeight = 0.0f;
                synapse->targetNodeId = UNINIT_STATE_16;
            } 
            else 
            {
                transferContainer.newWeight += synapseWeight;
            }
        }

        // create update-container for the host
        transferContainer.targetId = synapseSection->sourceEdgeId;
        transferContainer.positionInEdge = synapseSection->positionInEdge;
        transferContainer.deleteEdge = transferContainer.newWeight <= localGlobalValue->deleteSynapseBorder;

        // delete +1 = DELETED_SECTION
        synapseSection->status = transferContainer.deleteEdge + 1;
        updateTransfers[i] = transferContainer;
        synapseSections[i] = tempSectionMem[localId_x];
    }
}
