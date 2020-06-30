#pragma OPENCL EXTENSION cl_khr_fp16 : enable

// const predefined values
#define UNINIT_STATE_64 0xFFFFFFFFFFFFFFFF
#define UNINIT_STATE_32 0xFFFFFFFF
#define UNINIT_STATE_24 0xFFFFFF
#define UNINIT_STATE_16 0xFFFF
#define UNINIT_STATE_8  0xFF

// common information
#define SYNAPSES_PER_SYNAPSESECTION 19
#define MAX_NUMBER_OF_NODES_PER_BRICK 16384
#define NUMBER_OF_NODES_PER_BRICK 1000

// learning
#define NEW_FORWARD_EDGE_BORDER 0.1f
#define NEW_SYNAPSE_BORDER 1.0f
#define DELETE_SYNAPSE_BORDER 1.0f
#define MINIMUM_NEW_EDGE_BODER 1.0f
#define MAX_SOMA_DISTANCE 5

// processing
#define NUMBER_OF_PROCESSING_UNITS 1
#define PROCESS_INTERVAL 100000 // usec
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
} SynapseTransfer;

//==================================================================================================

typedef struct AxonTransfer_struct
{
    uint targetId;
    ulong path;
    float weight;
} AxonTransfer;

//==================================================================================================

typedef struct UpdateTransfer_struct
{
    uint brickId ;
    uint targetId ;
    uchar deleted ;
    float weightDiff;
} UpdateTransfer;

//==================================================================================================

typedef struct RandTransfer_struct
{
    float randWeight[999];
    uint randPos[1024];
} RandTransfer;

//==================================================================================================

typedef struct Node_struct
{
    float currentState;
    float border;

    float potential;
    uchar refractionTime;
    uchar padding[5];

    uchar active;
    uchar tooHigh;

    // Axon
    ulong targetBrickPath;
    uint targetAxonId;

} Node;

//==================================================================================================

typedef struct Synapse_struct
{
    float weight;
    float memorize;
    ushort targetNodeId;
    uchar inProcess;
    uchar somaDistance;

} Synapse;

//==================================================================================================

typedef struct SynapseSection_struct
{
    uchar status;

    uchar numberOfSynapses;
    uchar padding[6];

    ulong activeMapping;
    float totalWeight;

    uint sourceEdgeId;
    uint sourceBrickId;

    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];
} SynapseSection;

//==================================================================================================

float
makePositive(const float input)
{
    float floatRep = input;
    uint* convertedValue = (uint*)(&floatRep);
    // delete sign-bit
    *convertedValue = 0x7FFFFFFF & *convertedValue;
    return *(float*)(convertedValue);
}

//==================================================================================================

/**
 * check if all slots of the section are filled
 *
 * @return true, if full, else false
 */
bool
isFull(__local SynapseSection* synapseSection)
{
    return synapseSection->numberOfSynapses >= SYNAPSES_PER_SYNAPSESECTION;
}

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
    if(synapseSection->numberOfSynapses < SYNAPSES_PER_SYNAPSESECTION)
    {
        Synapse newSynapse;

        newSynapse.targetNodeId = (ushort)(targetNodeId % NUMBER_OF_NODES_PER_BRICK);
        newSynapse.memorize = globalMemorizingOffset;
        newSynapse.somaDistance = (uchar)((somaDistance % (MAX_SOMA_DISTANCE - 1)) + 1);

        const uint pos = synapseSection->numberOfSynapses;
        synapseSection->synapses[pos] = newSynapse;
        synapseSection->numberOfSynapses++;
    }
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
    synapseSection->activeMapping = 0;
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
        float diff = makePositive(synapseSection->synapses[position].weight);
        synapseSection->synapses[position].weight += weightUpdate;
        diff -= makePositive(synapseSection->synapses[position].weight);
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

    const uint targetNodeId = randTransfers->randPos[(index) % 1024];
    const uint somaDistance = randTransfers->randPos[(index + 1) % 1024];

    addSynapse(synapseSection,
               0.5f,
               targetNodeId,
               somaDistance);
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

    ulong index = get_global_id(0) * get_local_id(0);
    index = index - (index % 3);

    for(uchar i = 0; i < 3; i++)
    {
        const uint choosePosition = randTransfers->randPos[index] 
                                    % (synapseSection->numberOfSynapses + 1);
        const float currentSideWeight = randTransfers->randWeight[index + i] * weight;

        // create new synapse if necessary
        if(choosePosition == synapseSection->numberOfSynapses) {
            createNewSynapse(synapseSection, randTransfers);
        }

        // synapses, which are fully memorized, are not allowed to be overwritten!!!
        if(synapseSection->synapses[choosePosition].memorize >= 0.99f) {
            continue;
        }

        // calculate new value
        const float newVal = 0.5 * currentSideWeight;

        synapseSection->synapses[choosePosition].weight += newVal;
        synapseSection->totalWeight += makePositive(newVal);
    }
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
    learningSynapseSection(synapseSection,
                           inputWeight - synapseSection->totalWeight,
                           randTransfers);

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
             ulong nodeIdInBrick,
             __global AxonTransfer* axonTransfers)
{
    // set to 255.0f, if value is too high
    if(node->currentState > 255.0f) {
        node->currentState = 255.0f;
    }

    // check if active
    //printf("border: %f    id-value: %f \n", node->border, node->currentState);
    if(node->border <= node->currentState
            && node->refractionTime == 0)
    {
        node->potential = ACTION_POTENTIAL;
        node->active = 1;
        node->refractionTime = REFRACTION_TIME;
    }
    else if(node->refractionTime == 0) {
        node->active = 0;
    }

    // forward current potential
    if(node->active == 1)
    {
        const float weight = node->potential * PROCESSING_MULTIPLICATOR;
        const ulong path = node->targetBrickPath/32;

        AxonTransfer newEdge;
        newEdge.targetId = node->targetAxonId;
        newEdge.path = path;
        newEdge.weight = weight;

        axonTransfers[nodeIdInBrick] = newEdge;
    }
    else
    {
        AxonTransfer newEdge;
        newEdge.targetId = 0;
        newEdge.path = 0;
        newEdge.weight = 0.0f;

        axonTransfers[nodeIdInBrick] = newEdge;
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

//==================================================================================================

/**
 * @brief memorizeEdges
 * @param brick
 */
bool
memorizeSynapses(__local SynapseSection* synapseSection,
                 ulong sectionPosition)
{
    // skip if section is deleted
    if(synapseSection->status != ACTIVE_SECTION) {
        return false;
    }

    // update values based on the memorizing-value
    __local Synapse* end = &synapseSection->synapses[synapseSection->numberOfSynapses];
    for(__local Synapse* synapse = synapseSection->synapses;
        synapse < end;
        synapse++)
    {
        if(synapse->inProcess == 1) {
            return false;
        }

        const float newWeight = synapse->weight * (1.0f - synapse->memorize);
        synapse->weight -= newWeight;
        synapseSection->totalWeight -= newWeight;
    }

    if(synapseSection->totalWeight < DELETE_SYNAPSE_BORDER) {
        return true;
    }

    return false;
}

//==================================================================================================

__kernel void 
processing(__global const SynapseTransfer* synapseTransfers,
           ulong numberOfSynapseTransfers,
           __global AxonTransfer* axonTransfers,
           ulong numberOfAxonTransfers,
           __global UpdateTransfer* updateTransfers,
           ulong numberOfUpdateTransfers,
           __global Node* nodes,
           ulong numberOfNodes,
           __global SynapseSection* synapseSections,
           ulong numberOfSynapseSections,
           __global RandTransfer* randTransfers,
           ulong numberOfRandTransfers)
{
    // calculate global ids
    size_t globalId_x = get_global_id(0);
    int localId_x = get_local_id(0);
    uint numberOfBricks = numberOfNodes / NUMBER_OF_NODES_PER_BRICK;
    uint brickId = globalId_x / 256; 

    // init shared memory
    __local uchar localMemory[256];

    // debug-prints
    if(globalId_x == 0)
    {
        printf("number of Nodes: %d\n", numberOfNodes);
        printf("number of SynapseTransfers: %d\n", numberOfSynapseTransfers);
        printf("number of AxonTransfers: %d\n", numberOfAxonTransfers);
        printf("number of UpdateTransfers: %d\n", numberOfUpdateTransfers);
        printf("number of SynapseSections: %d\n", numberOfSynapseSections);
    }

    if(brickId >= numberOfBricks) {
        return;
    }

    //----------------------------------------------------------------------------------------------
    // process input synapses
    //----------------------------------------------------------------------------------------------
    __local SynapseSection* tempSections = (__local SynapseSection*)localMemory;

    for(uint i = localId_x; i < numberOfSynapseSections; i = i + 256)
    {
        if(synapseTransfers[i].brickId == UNINIT_STATE_32
            || synapseTransfers[i].brickId != brickId) 
        {
            continue;
        }

        tempSections[0] = synapseSections[i];

        if(tempSections[0].status == DELETED_SECTION) 
        {
            resetSynapseSection(tempSections,
                                synapseTransfers[i].brickId,
                                synapseTransfers[i].sourceEdgeId);
        }
        else
        {
            processSynapseSection(tempSections,
                                  nodes,
                                  synapseTransfers[i].weight,
                                  randTransfers);
        }

        synapseSections[i] = tempSections[0];
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    //----------------------------------------------------------------------------------------------
    // process nodes
    //----------------------------------------------------------------------------------------------
    __local Node* tempNodes = (__local Node*)&localMemory[0];

    for(uint i = localId_x; i < NUMBER_OF_NODES_PER_BRICK; i = i + 256)
    {
        tempNodes[0] = nodes[i];
        processNodes(tempNodes, i, axonTransfers);
        nodes[i] = tempNodes[0];
    }

    work_group_barrier(CLK_LOCAL_MEM_FENCE);

    //----------------------------------------------------------------------------------------------
    // process memorizing
    //----------------------------------------------------------------------------------------------
    __local SynapseSection* tempSectionMem = (__local SynapseSection*)&localMemory[0];

    for(uint i = localId_x; i < numberOfSynapseSections; i = i + 256)
    {
        tempSectionMem[0] = synapseSections[i];

        if(tempSectionMem->sourceBrickId != brickId) {
            continue;
        }

        // TODO: memorizing

        synapseSections[i] = tempSectionMem[0];
    }
    //----------------------------------------------------------------------------------------------
}
