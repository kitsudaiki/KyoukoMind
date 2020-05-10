#define SYNAPSES_PER_SYNAPSESECTION 64
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
    uint targetId;
    float weight;
} SynapseTransfer __attribute__((packed));

//==================================================================================================

typedef struct AxonTransfer_struct
{
    uint targetId;
    ulong path;
    float weight;
} AxonTransfer __attribute__((packed));

//==================================================================================================

typedef struct Node_struct
{
    float currentState;
    float border;

    float potential;
    uchar refractionTime;

    uchar active;
    uchar tooHigh;

    // Axon
    ulong targetBrickPath;
    uint targetAxonId;
} Node __attribute__((packed));

//==================================================================================================

typedef struct Synapse_struct
{
    float weight;
    ushort targetNodeId;
    float memorize;
    uchar inProcess;
    uchar somaDistance;

} Synapse __attribute__((packed));

//==================================================================================================

typedef struct SynapseSection_struct
{
    uchar status;

    uchar numberOfSynapses;
    ulong activeMapping;
    float totalWeight;

    uint sourceId;
    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];
} SynapseSection __attribute__((packed));

//==================================================================================================

/**
 * processing of the nodes of a specific node-brick
 *
 * @return number of active nodes in this brick
 */
void
processNodes(__global Node* node,
             ulong numberOfNode,
             __global AxonTransfer* axonTransfers,
             __local Node* tempNode)
{
    // set to 255.0f, if value is too high
    if(node->currentState > 255.0f) {
        node->currentState = 255.0f;
    }

    // init
    *tempNode = *node;

    // check if active
    if(tempNode->border <= tempNode->currentState
            && tempNode->refractionTime == 0)
    {
        node->potential = ACTION_POTENTIAL;
        node->active = 1;
        node->refractionTime = REFRACTION_TIME;
    }
    else if(tempNode->refractionTime == 0) {
        node->active = 0;
    }

    // forward current potential
    if(node->active == 1)
    {
        const float weight = node->potential * PROCESSING_MULTIPLICATOR;
        const ulong path = tempNode->targetBrickPath/32;

        AxonTransfer newEdge;
        newEdge.targetId = tempNode->targetAxonId;
        newEdge.path = path;
        newEdge.weight = weight;

        axonTransfers[numberOfNode] = newEdge;
    }
    else
    {
        AxonTransfer newEdge;
        newEdge.targetId = 0;
        newEdge.path = 0;
        newEdge.weight = 0.0f;

        axonTransfers[numberOfNode] = newEdge;
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
memorizeSynapses(SynapseSection* synapseSection,
                 ulong sectionPosition)
{
    // skip if section is deleted
    if(synapseSection->status != ACTIVE_SECTION) {
        return false;
    }

    // update values based on the memorizing-value
    Synapse* end = synapseSection->synapses + synapseSection->numberOfSynapses;
    for(Synapse* synapse = synapseSection->synapses;
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

__kernel void processing(
       __global const SynapseTransfer* synapseTransfers,
       ulong numberOfSynapseTransfers,
       __global AxonTransfer* axonTransfers,
       ulong numberOfAxonTransfers,
       __global Node* nodes,
       ulong numberOfNodes,
       __global SynapseSection* synapseSections,
       ulong numberOfSynapseSections
    )
{
    __local Node tempNodes[256];

    size_t globalId_x = get_global_id(0);
    int localId_x = get_local_id(0);

    for(uint i = 0; i < NUMBER_OF_NODES_PER_BRICK; i=i+256)
    {
        processNodes(&nodes[i], i, axonTransfers, &tempNodes[localId_x]);
    }
}
