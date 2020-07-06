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
#define PROCESSING_MULTIPLICATOR 1.0f

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
    uint sourceEdgeId;
    float weight;
} 
SynapseTransfer;

//==================================================================================================

typedef struct AxonTransfer_struct
{
    uint targetId;
    ulong path;
    float weight;
} 
AxonTransfer;

//==================================================================================================

typedef struct UpdateTransfer_struct
{
    uint brickId;
    uint targetId;
    uchar deleted;
    float weightDiff;
} 
UpdateTransfer;

//==================================================================================================

typedef struct RandTransfer_struct
{
    float randWeight[999];
    uint randPos[1024];
} 
RandTransfer;

//==================================================================================================

typedef struct Node_struct
{
    float currentState;
    float border;

    float potential;
    uchar refractionTime;
    uchar active;

    uchar padding[6];

    // Axon
    ulong targetBrickPath;
    uint targetAxonId;
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

    uchar numberOfSynapses;
    uchar padding[2];

    float totalWeight;

    uint sourceEdgeId;
    uint sourceBrickId;

    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];
} 
SynapseSection;

//==================================================================================================

/**
 * add a new synapse to the current section
 *
 * @return false, if the section is already full, else true
 */
void
addSynapse(__local SynapseSection* synapseSection,
           const float globalMemorizingOffset,
           const uint targetNodeId,
           const uint somaDistance)
{
    if(synapseSection->numberOfSynapses >= SYNAPSES_PER_SYNAPSESECTION) {
        return;
    }

    Synapse newSynapse;
    newSynapse.targetNodeId = (ushort)(targetNodeId % NUMBER_OF_NODES_PER_BRICK);
    newSynapse.memorize = globalMemorizingOffset;
    newSynapse.somaDistance = (uchar)((somaDistance % (MAX_SOMA_DISTANCE - 1)) + 1);

    const uint pos = synapseSection->numberOfSynapses;
    synapseSection->synapses[pos] = newSynapse;
    synapseSection->numberOfSynapses++;
}

//==================================================================================================

/**
 * @return false, if the section is already full, else true
 */
void
resetSynapseSection(__local SynapseSection* synapseSection,
                    const uint sourceEdgeId,
                    const uint sourceBrickId)
{
    synapseSection->status = ACTIVE_SECTION;

    synapseSection->numberOfSynapses = 0;
    synapseSection->totalWeight = 0.0000001f;

    synapseSection->sourceEdgeId = sourceEdgeId;
    synapseSection->sourceBrickId = sourceBrickId;
}

//==================================================================================================
/**
 * @brief updateSynapseWeight
 * @param section
 * @param position
 * @return
 */
void
updateSynapseWeight(__local SynapseSection* synapseSection,
                    const uint position,
                    const float weightUpdate)
{
    if(position < synapseSection->numberOfSynapses)
    {
        float diff = fabs(synapseSection->synapses[position].weight);
        synapseSection->synapses[position].weight += weightUpdate;
        diff -= fabs(synapseSection->synapses[position].weight);
        synapseSection->totalWeight -= diff;
    }
}

//==================================================================================================

/**
 * @brief NodeBrick::createNewEdge
 * @param currentSection
 * @param edgeSectionId
 */
void
createNewSynapse(__local SynapseSection* synapseSection,
                 __global RandTransfer* randTransfers)
{
    const ulong index = get_global_id(0) * get_local_id(0);

    const uint targetNodeId = randTransfers->randPos[(index) % 1024] % NUMBER_OF_NODES_PER_BRICK;
    const uint somaDistance = randTransfers->randPos[(index + 1) % 1024] % 256;

    addSynapse(synapseSection,
               0.5f,  // memorizing
               targetNodeId,
               somaDistance);
}

//==================================================================================================

void
singleLearningStep(__local SynapseSection* synapseSection,
                   const float weight,
                   __global RandTransfer* randTransfers,
                   const uint index)
{
    const uint posRandArry = (get_global_id(0) % 333) * 3 + index;
    const uint maxPos = (synapseSection->numberOfSynapses + 1) % SYNAPSES_PER_SYNAPSESECTION;
    const uint choosePosition = (randTransfers->randPos[posRandArry] 
                                 % (synapseSection->numberOfSynapses + 1))
                                    % maxPos;

    const float currentSideWeight = randTransfers->randWeight[posRandArry] * weight;

    // create new synapse if necessary
    if(choosePosition == synapseSection->numberOfSynapses) {
        createNewSynapse(synapseSection, randTransfers);
    }

    // synapses, which are fully memorized, are not allowed to be overwritten!!!
    if(synapseSection->synapses[choosePosition].memorize >= 0.99f) {
        return;
    }

    // calculate new value
    // TODO: add lerning-value
    synapseSection->synapses[choosePosition].weight += currentSideWeight;
    synapseSection->totalWeight += fabs(currentSideWeight);
}

//==================================================================================================

/**
 * learing-process of the specific synapse-section
 *
 * @param currentSection synapse-section with should learn the new value
 * @param weight weight-difference to learn
 */
void
learningSynapseSection(__local SynapseSection* synapseSection,
                       float weight,
                       __global RandTransfer* randTransfers)
{
    if(weight < NEW_SYNAPSE_BORDER) {
        return;
    }

    singleLearningStep(synapseSection, weight, randTransfers, 0);
    singleLearningStep(synapseSection, weight, randTransfers, 1);
    singleLearningStep(synapseSection, weight, randTransfers, 2);
}

//==================================================================================================

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
                      __global RandTransfer* randTransfers)
{
    if(inputWeight < 0.000001f) {
        return;
    }

    // run lerning-process by creating and updating synapses
    const float weightDiff = inputWeight - synapseSection->totalWeight;
    learningSynapseSection(synapseSection, weightDiff, randTransfers);

    if(synapseSection->totalWeight < 0.000001f) {
        return;
    }

    // limit ration to 1.0f
    float ratio = inputWeight / synapseSection->totalWeight;
    if(ratio > 1.0f) {
        ratio = 1.0f;
    }


    __local Synapse* end = synapseSection->synapses + synapseSection->numberOfSynapses;

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

//==================================================================================================

/**
 * processing of the nodes of a specific node-brick
 *
 * @return number of active nodes in this brick
 */
void
processNodes(__local Node* node,
             ulong globalNodeId,
             __global AxonTransfer* axonTransfers)
{
    // set to 255.0f, if value is too high
    if(node->currentState > 255.0f) {
        node->currentState = 255.0f;
    }
    if(node->currentState < 0.000001f) {
        node->currentState = 0.0f;
    }

    // check if active
    if(node->border <= node->currentState
            && node->refractionTime == 0)
    {
        printf("ACTIVE\n");
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
    newEdge.targetId = node->active * node->targetAxonId;
    newEdge.path = node->active * (node->targetBrickPath/32);
    newEdge.weight = node->active * (node->potential * PROCESSING_MULTIPLICATOR);
    axonTransfers[globalNodeId] = newEdge;

    // post-steps
    node->refractionTime = node->refractionTime >> 1;

    // set to 0.0f, if value is negative
    if(node->currentState < 0.0f) {
        node->currentState = 0.0f;
    }

    // make cooldown in the node
    node->potential /= NODE_COOLDOWN;
    node->currentState /= NODE_COOLDOWN;
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
    // skip if section is deleted
    if(synapseSection->status != ACTIVE_SECTION) {
        return;
    }

    // update values based on the memorizing-value
    __local Synapse* end = synapseSection->synapses + synapseSection->numberOfSynapses;

    for(__local Synapse* synapse = synapseSection->synapses;
        synapse < end;
        synapse++)
    {
        if(synapse->inProcess == 1) {
            return;
        }

        const float diff = synapse->weight * (1.0f - synapse->memorize);
        synapse->weight -= diff;
        synapseSection->totalWeight -= fabs(diff);

        UpdateTransfer transferContainer;
        transferContainer.brickId = synapseSection->sourceBrickId;
        transferContainer.targetId = synapseSection->sourceEdgeId;
        transferContainer.weightDiff = fabs(diff);
        transferContainer.deleted = false;

        if(synapseSection->totalWeight <= DELETE_SYNAPSE_BORDER) 
        {
            transferContainer.deleted = true;
            synapseSection->status = DELETED_SECTION;
            resetSynapseSection(synapseSection,
                                synapseSection->sourceBrickId,
                                synapseSection->sourceEdgeId);
        }

        updateTransfers[sectionPosition] = transferContainer;

    }
}

//==================================================================================================

__kernel void 
processing(__global const SynapseTransfer* synapseTransfers,
           const ulong numberOfSynapseTransfers,
           __global AxonTransfer* axonTransfers,
           const ulong numberOfAxonTransfers,
           __global UpdateTransfer* updateTransfers,
           const ulong numberOfUpdateTransfers,
           __global Node* nodes,
           const ulong numberOfNodes,
           __global SynapseSection* synapseSections,
           const ulong numberOfSynapseSections,
           __global RandTransfer* randTransfers,
           const ulong numberOfRandTransfers,
           __local uchar* localMemory,
           const ulong localMemorySize)
{
    // calculate global ids
    size_t globalId_x = get_global_id(0);
    size_t globalSize_x = get_global_size(0);
    int localId_x = get_local_id(0);
    int localSize_x = get_local_size(0);

    uint numberOfBricks = numberOfNodes / NUMBER_OF_NODES_PER_BRICK;
    uint brickId = globalId_x / localSize_x; 

    // debug-prints
    if(globalId_x == 0) {
        printf("numberOfSynapseTransfers: %d\n", numberOfSynapseTransfers);
    }

    if(brickId >= numberOfBricks) {
        return;
    }

    //----------------------------------------------------------------------------------------------
    // process input synapses
    //----------------------------------------------------------------------------------------------
    __local SynapseSection* tempSections = (__local SynapseSection*)localMemory;

    for(ulong i = localId_x; i < numberOfSynapseTransfers; i = i + localSize_x)
    {
        if(synapseTransfers[i].brickId != brickId) {
            continue;
        }

        tempSections[localId_x] = synapseSections[synapseTransfers[i].sourceEdgeId];

        processSynapseSection(&tempSections[localId_x],
                              nodes,
                              synapseTransfers[i].weight,
                              randTransfers);

        synapseSections[i] = tempSections[localId_x];
        synapseSections[i].sourceBrickId = brickId;
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    //----------------------------------------------------------------------------------------------
    // process nodes
    //----------------------------------------------------------------------------------------------
    __local Node* tempNodes = (__local Node*)localMemory;

    for(ulong i = globalId_x; i < numberOfNodes; i = i + globalSize_x)
    {
        tempNodes[localId_x] = nodes[i];
        processNodes(&tempNodes[localId_x], 
                     i, 
                     axonTransfers);
        nodes[i] = tempNodes[localId_x];
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    //----------------------------------------------------------------------------------------------
    // process memorizing
    //----------------------------------------------------------------------------------------------
    __local SynapseSection* tempSectionMem = (__local SynapseSection*)localMemory;

    for(uint i = globalId_x; i < numberOfSynapseSections; i = i + globalSize_x)
    {
        tempSectionMem[localId_x] = synapseSections[i];
        memorizeSynapses(&tempNodes[localId_x], 
                         i, 
                         updateTransfers);
        synapseSections[i] = tempSectionMem[localId_x];
    }
    //----------------------------------------------------------------------------------------------
}
