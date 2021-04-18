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
#define SYNAPSES_PER_SYNAPSESECTION 62
#define OUTPUT_SYNAPSES_PER_SECTION 4094


typedef struct NetworkMetaData_struct
{
    float lerningValue = 0.0f;
    uint cycleTime = 1000000;
    uchar doLearn = 0;

    uchar padding[247];
}
NetworkMetaData;

typedef struct SynapseMetaData_struct
{
    float synapseDeleteBorder = 1.0f;
    float actionPotential = 100.0f;
    float nodeCooldown = 100.0f;
    float memorizing = 0.1f;
    float gliaValue = 1.0f;
    float signNeg = 0.6f;
    float potentialOverflow = 20.0f;
    float maxSynapseWeight = 30.0f;
    uchar refractionTime = 1;
    uchar multiplicatorRange = 0;

    uchar padding[222];
}
SynapseMetaData;

typedef struct OutputMetaData_struct
{
    float lowerMatch = 0.0f;
    float upperMatch = 0.0f;
    uint inputOffset = 0;
    uint inputRange = 0;
    uint maxConnections = 0;

    uchar padding[236];
}
OutputMetaData;

//==================================================================================================

typedef struct SynapseBufferEntry_struct
{
    float weigth;
    uint nodeId;
    // total size: 8 Byte
}
SynapseBufferEntry;

//==================================================================================================

typedef struct SynapseBuffer_struct
{
    uchar process;
    uchar upToDate;
    uchar padding[6];

    SynapseBufferEntry buffer[8];

    // total size: 64 Byte
}
SynapseBuffer;

//==================================================================================================

typedef struct Synapse_struct
{
    float weight;
    ushort targetNodeId;
    char sign;
    char multiplicator;
    // total size: 8 Byte
}
Synapse;

//==================================================================================================

typedef struct SynapseSection_struct
{
    ushort active;
    ushort randomPos;

    uint brickBufferPos;
    uint hardening;
    uchar padding[4];

    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];

    // total size: 512 Byte
}
SynapseSection;

//==================================================================================================

typedef struct  InputNode_struct
{
    float weight;
    uint targetNode;
}
InputNode;

//==================================================================================================

typedef struct Node_struct
{
    float currentState;
    float border;
    float potential;

    uchar refractionTime;
    uchar active;
    uchar upToDate;
    uchar external;

    uint nodeBrickId;
    uint targetBrickDistance;

    uint targetSectionId;
    uchar padding2[4];

    // total size: 32 Byte
}
Node;

//==================================================================================================

typedef struct OutputSynapse_struct
{
    float border;
    float weight;
    uint targetId;
    uchar newOne;
    uchar active;
    uchar padding[2];
    // total size: 16 Byte
}
OutputSynapse;

//==================================================================================================

typedef struct OutputSynapseSection_struct
{
    ushort status;
    ushort randomPos;

    ulong prev;

    uint hardening ;

    uint newOnes;
    uint total;

    float diffNew;
    float diffTotal;

    OutputSynapse synapses[OUTPUT_SYNAPSES_PER_SECTION];

    // total size: 4096 Byte
}
OutputSynapseSection;

//==================================================================================================

typedef struct Output_struct
{
    float outputValue;
    float shouldValue;
}
Output;

//==================================================================================================

enum SegmentType
{
    UNDEFINED_SEGMENT = 0,
    SYNAPSE_SEGMENT = 1,
    OUTPUT_SEGMENT = 2
};

//==================================================================================================

typedef struct SynapseSegmentMeta_struct
{
    uint segmentType = UNDEFINED_SEGMENT;

    // synapse-segment
    uint numberOfNodeBricks;
    ulong numberOfSynapseSections;
    ulong numberOfNodes;
    uint numberOfNodesPerBrick;

    // generic
    uint numberOfRandomValues;
    uint numberOfInputs;

    uchar padding[224];
}
SynapseSegmentMeta;

//==================================================================================================

typedef struct OutputSegmentMeta_struct
{
    uint segmentType;

    // output-segment
    uint numberOfOutputs;

    // generic
    uint numberOfRandomValues;
    uint numberOfInputs;

    uchar padding[224];
}
OutputSegmentMeta;

//==================================================================================================

typedef struct Position_struct
{
    int x = 0;
    int y = 0;
    int z = 0;
    int w = 0;
}
Position;

//==================================================================================================

typedef struct Brick_struct
{
    // common
    uint brickId = UNINIT_STATE_32;
    uint nodeBrickId = UNINIT_STATE_32;
    uint layerId = UNINIT_STATE_32;
    bool isOutputBrick = false;
    bool isInputBrick = false;
    uchar padding[10];

    Position brickPos;

    uint neighbors[12];
    uint possibleTargetNodeBrickIds[1000];
    uint nodePos = UNINIT_STATE_32;

    uint nodeActivity = 0;
    // total size: 4096 Bytes
}
Brick;

//==================================================================================================
