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
#define SYNAPSES_PER_SYNAPSESECTION 20
#define NUMBER_OF_NODES_PER_BRICK 1000

// learning
#define NEW_SYNAPSE_BORDER 1.0f
#define DELETE_SYNAPSE_BORDER 1.0f
#define MAX_SOMA_DISTANCE 5

// processing
#define NODE_COOLDOWN 3.0f
#define ACTION_POTENTIAL 100.0f
#define REFRACTION_TIME 2

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
    float weight;
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
    float globalMemorizingOffset;
    float globalMemorizingTemp;

    float globalLearningOffset;
    float globalLearningTemp;

    float globalOutLearningOffset;
    float globalOutLearningTemp;

    float globalGlia;

    uint runUpdate;
}
GlobalValues;

//==================================================================================================

void
singleLearningStep(__local SynapseSection* synapseSection,
                   const float weight,
                   __global uint* randomInts)
{
    synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
    const uint choosePosition = randomInts[synapseSection->randomPos] % SYNAPSES_PER_SYNAPSESECTION;

    __local Synapse* chosenSynapse = &synapseSection->synapses[choosePosition];

    // create new synapse if necessary
    if(chosenSynapse->targetNodeId == UNINIT_STATE_16
        || fabs(chosenSynapse->weight) < 0.01f) 
    {
        synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
        const uint targetNodeId = randomInts[synapseSection->randomPos] % NUMBER_OF_NODES_PER_BRICK;
        synapseSection->randomPos = (synapseSection->randomPos + 1) % 1024;
        const uint somaDistance = randomInts[synapseSection->randomPos] % 256;

        chosenSynapse->targetNodeId = (ushort)(targetNodeId % NUMBER_OF_NODES_PER_BRICK);
        chosenSynapse->memorize = 0.5f;  // memorizing
        chosenSynapse->somaDistance = (uchar)((somaDistance % (MAX_SOMA_DISTANCE - 1)) + 1);
        chosenSynapse->weight = 0.0f;
    }

    // synapses, which are fully memorized, are not allowed to be overwritten!!!
    if(chosenSynapse->memorize >= 0.99f) {
        return;
    }

    // calculate new value
    // TODO: add lerning-value
    synapseSection->synapses[choosePosition].weight += weight;
    synapseSection->totalWeight += fabs(weight);
}

//--------------------------------------------------------------------------------------------------

/**
* process of a specific edge-section of a brick
*
* @param edgeSectionId id of the edge-section within the current brick
* @param weight incoming weight-value
*/
void
processSynapseSection(__local SynapseSection* synapseSection,
                      __global Node* nodes,
                      const float inputWeight,
                      __global uint* randomInts)
{
    if(inputWeight < 0.000001f) {
        return;
    }

    // run lerning-process by creating and updating synapses
    const float weightDiff = inputWeight - synapseSection->totalWeight;
    if(weightDiff > 0.0f)
    {
        singleLearningStep(synapseSection, 0.5f * weightDiff, randomInts);
        singleLearningStep(synapseSection, 0.3f * weightDiff, randomInts);
        singleLearningStep(synapseSection, 0.2f * weightDiff, randomInts);
    }

    // limit ration to 1.0f
    float ratio = inputWeight / synapseSection->totalWeight;
    ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

    __local Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;

    for(__local Synapse* synapse = synapseSection->synapses;
        synapse < end;
        synapse++)
    {
        const Synapse tempSynapse = *synapse;
        nodes[tempSynapse.targetNodeId].currentState +=
                tempSynapse.weight
                * ratio
                * ((float)tempSynapse.somaDistance / (float)MAX_SOMA_DISTANCE);
        synapse->inProcess = nodes[tempSynapse.targetNodeId].active;
    }
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
    const uint numberOfBricks = numberOfNodes / NUMBER_OF_NODES_PER_BRICK;
    const uint brickId = globalId_x / localSize_x; 

    globalValue->globalGlia = 1.1f;

    if(brickId >= numberOfBricks) {
        return;
    }

    __local SynapseSection* tempSections = (__local SynapseSection*)localMemory;

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

        if(tempSections[localId_x].status != ACTIVE_SECTION) {
            continue;
        }

        processSynapseSection(&tempSections[localId_x],
                              nodes,
                              synapseTransfers[i].weight,
                              randomInts);

        synapseSections[synapseSectionId] = tempSections[localId_x];
        synapseSections[synapseSectionId].sourceBrickId = brickId;
    }
}

//==================================================================================================

/**
 * processing of the nodes of a specific node-brick
 *
 * @return number of active nodes in this brick
 */
void
processNodes(__local Node* node,
             ulong globalNodeId,
             __global AxonTransfer* axonTransfers,
             __global GlobalValues* globalValue)
{
    // set to 255.0f, if value is too high
    const float cur = node->currentState;
    node->currentState = (cur > 255.0f) * 255.0f + (cur <= 255.0f) * cur;
    node->currentState = (cur < 0.000001f) * 0.0f + (cur >= 0.000001f) * cur;

    // check if active
    if(node->border <= node->currentState
            && node->refractionTime == 0)
    {
        node->potential = ACTION_POTENTIAL;
        node->active = 1;
        node->refractionTime = REFRACTION_TIME;
    }
    else if(node->refractionTime == 0) 
    {
        node->active = 0;
    }


    // build new axon-transfer-edge, which is send back to the host
    AxonTransfer newEdge;
    newEdge.weight = node->active * node->potential * pow(globalValue->globalGlia, node->targetBrickDistance);
    axonTransfers[globalNodeId] = newEdge;

    // post-steps
    node->refractionTime = node->refractionTime >> 1;

    // set to 0.0f, if value is negative
    const float newCur = node->currentState;
    node->currentState = (newCur < 0.0f) * 0.0f + (newCur >= 0.0f) * newCur;

    // make cooldown in the node
    node->potential /= NODE_COOLDOWN;
    node->currentState /= NODE_COOLDOWN;
}

//--------------------------------------------------------------------------------------------------

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
    const uint numberOfBricks = numberOfNodes / NUMBER_OF_NODES_PER_BRICK;
    const uint brickId = globalId_x / localSize_x; 

    __local Node* tempNodes = (__local Node*)localMemory;

    if(brickId >= numberOfBricks) {
        return;
    }

    for(ulong i = globalId_x; i < numberOfNodes; i = i + globalSize_x)
    {
        tempNodes[localId_x] = nodes[i];
        processNodes(&tempNodes[localId_x],
                     i,
                     axonTransfers,
                     globalValue);
        nodes[i] = tempNodes[localId_x];
    }
}

//==================================================================================================

/**
 * @brief memorizeEdges
 * @param brick
 */
void
memorizeSynapses(__local SynapseSection* synapseSection,
                 const ulong sectionPosition,
                 __global UpdateTransfer* updateTransfers)
{
    // update values based on the memorizing-value
    __local Synapse* end = synapseSection->synapses + SYNAPSES_PER_SYNAPSESECTION;

    float totalWeight = 0.0f;

    for(__local Synapse* synapse = synapseSection->synapses;
        synapse < end;
        synapse++)
    {
        // update weight
        synapse->weight -= synapse->weight * (1.0f - synapse->memorize);
        totalWeight += fabs(synapse->weight);
    }

    // create update-container for the host
    UpdateTransfer transferContainer;
    transferContainer.targetId = synapseSection->sourceEdgeId;
    transferContainer.positionInEdge = synapseSection->positionInEdge;
    transferContainer.newWeight = totalWeight;
    transferContainer.deleteEdge = totalWeight <= DELETE_SYNAPSE_BORDER;

    synapseSection->status = transferContainer.deleteEdge + 1;

    updateTransfers[sectionPosition] = transferContainer;
}

//--------------------------------------------------------------------------------------------------

__kernel void
updating(__global UpdateTransfer* updateTransfers,
         const ulong numberOfUpdateTransfers,
         __global SynapseSection* synapseSections,
         const ulong numberOfSynapseSections,
         __local uchar* localMemory,
         const ulong localMemorySize)
{
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);

    __local SynapseSection* tempSectionMem = (__local SynapseSection*)localMemory;

    for(uint i = globalId_x; i < numberOfSynapseSections; i = i + globalSize_x)
    {
        // skip if section is deleted
        if(synapseSections[i].status == ACTIVE_SECTION)
        {
            tempSectionMem[localId_x] = synapseSections[i];
            memorizeSynapses(&tempSectionMem[localId_x],
                             i,
                             updateTransfers);
            synapseSections[i] = tempSectionMem[localId_x];
        }
        else
        {
            UpdateTransfer transferContainer;
            transferContainer.newWeight = -1.0f;
            transferContainer.targetId = UNINIT_STATE_32;
            transferContainer.positionInEdge = UNINIT_STATE_8;
            transferContainer.deleteEdge = 0;
            updateTransfers[i] = transferContainer;
        }
    }
}
