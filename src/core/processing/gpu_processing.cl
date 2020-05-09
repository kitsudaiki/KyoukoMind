#define SYNAPSES_PER_SYNAPSESECTION 64

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

__kernel void processing(
       __global const SynapseTransfer* synapseTransfers,
       ulong numberOfSynapseTransfers,
       __global AxonTransfer* axonTransfers,
       ulong numberOfAxonTransfers,
       __global const Node* nodes,
       ulong numberOfNodes,
       __global const SynapseSection* synapseSections,
       ulong numberOfSynapseSections
    )
{
    __local nodeStates[16384];

    size_t i = get_global_id(0);
    if (i < numberOfSynapseTransfers) {
       axonTransfers[i] = axonTransfers[i];
    }
}
