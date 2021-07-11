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

//#pragma OPENCL EXTENSION cl_khr_fp16 : enable

// const predefined values
#define UNINIT_STATE_64 0xFFFFFFFFFFFFFFFF
#define UNINIT_STATE_32 0xFFFFFFFF
#define UNINIT_STATE_24 0xFFFFFF
#define UNINIT_STATE_16 0xFFFF
#define UNINIT_STATE_8  0xFF

// common information
#define SYNAPSES_PER_SYNAPSESECTION 62
#define NUMBER_OF_PROCESSING_UNITS 1
#define BORDER_STEP (1.0f / 255.0f);
#define NUMBER_OF_RAND_VALUES 10485760
#define RAND_MAX 2147483647

typedef struct NetworkMetaData
{
    float lerningValue;
    uint cycleTime;
    uchar doLearn;

    uchar padding[247];
}
NetworkMetaData;

//==================================================================================================

typedef struct SynapseBuffer
{
    uchar process;
    uchar upToDate;
    uchar padding[2];

    float weigth;
    uint nodeId;
    uint targetId;

    // total size: 64 Byte
}
SynapseBuffer;

//==================================================================================================

typedef struct Synapse
{
    float weight;
    ushort targetNodeId;
    char activeCounter;
    uchar border;
}
Synapse;

//==================================================================================================

typedef struct SynapseSection
{
    uchar active;
    uchar padding1[3];
    uint randomPos;

    uint brickBufferPos;
    uint hardening;

    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];

    // total size: 512 Byte
}
SynapseSection;

//==================================================================================================

typedef struct InputNode
{
    float weight;
    uint targetNode;
}
InputNode;

//==================================================================================================

typedef struct OutputNode
{
    float outputWeight;
    float shouldValue;
    uint targetNode;
    uchar padding[4];
}
OutputNode;

//==================================================================================================

typedef struct Node
{
    float input;
    float border;
    float potential;

    float delta;

    uchar refractionTime;
    uchar active;
    uchar upToDate;
    uchar init;

    uint nodeBrickId;
    uint targetBrickDistance;

    uint targetSectionId;
    // total size: 32 Byte
}
Node;

//==================================================================================================

typedef struct Position
{
    int x;
    int y;
    int z;
    int w;
}
Position;

//==================================================================================================

typedef struct Brick
{
    // common
    uint brickId;
    uint nodeBrickId;
    uint layerId;
    bool isOutputBrick;
    bool isInputBrick;
    uchar padding[6];
    uint nodeActivity;

    Position brickPos;

    uint neighbors[12];
    uint possibleTargetNodeBrickIds[1000];
    uint nodePos;

    uint numberOfNodes;
    // total size: 4096 Bytes
}
Brick;

//==================================================================================================

typedef struct SegmentSettings
{
    float synapseDeleteBorder;
    float actionPotential;
    float nodeCooldown;
    float memorizing;
    float gliaValue;
    float signNeg;
    float potentialOverflow;
    float maxSynapseWeight;
    uchar refractionTime;
    uchar multiplicatorRange;
    uchar doLearn;

    uchar padding[221];
}
SegmentSettings;

typedef struct SegmentHeaderEntry
{
    ulong bytePos;
    ulong count;

    // total size: 16 Byte
} 
SegmentHeaderEntry;

typedef struct SegmentHeader
{
    ulong segmentSize;
    ulong segmentPersistentBufferSize;
    ulong segmentEphemeralBufferSize;

    // synapse-segment
    SegmentHeaderEntry settings;
    SegmentHeaderEntry bricks;
    SegmentHeaderEntry brickOrder;
    SegmentHeaderEntry synapseSections;
    SegmentHeaderEntry synapseBuffers;
    SegmentHeaderEntry nodes;
    SegmentHeaderEntry nodeBuffers;
    SegmentHeaderEntry inputs;
    SegmentHeaderEntry outputs;

    uchar padding[88];

    // total size: 256 Byte
} 
SegmentHeader;

typedef struct Segment
{
    __global SegmentHeader* segmentHeader;
    __global SegmentSettings* synapseSettings;
    __global Brick* bricks;
    __global uint* brickOrder;
    __global Node* nodes;
    __global float* nodeBuffers;
    __global SynapseSection* synapseSections;
    __global SynapseBuffer* synapseBuffers;
    __global InputNode* inputs;
    __global OutputNode* outputs;
} 
Segment;

//==================================================================================================


/**
 * @brief backpropagateNodes
 * @param brick
 * @param nodes
 * @param synapseSections
 */
inline void
backpropagateNodes(__global Brick* brick,
                   Segment* segment)
{
    for(uint nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        __global SynapseSection* section = &segment->synapseSections[nodeId];
        if(section->active == 0) {
            continue;
        }

        ushort pos = 0;
        __global Node* sourceNode = &segment->nodes[nodeId];
        float netH = sourceNode->potential;
        const float outH = 1.0f / (1.0f + exp(-1.0f * netH));

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            __global Synapse* synapse = &section->synapses[pos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }

            // update weight
            const float learnValue = (float)(pos <= section->hardening) * 0.1f
                                     + (float)(pos > section->hardening) * 0.4f;
            sourceNode->delta += segment->nodes[synapse->targetNodeId].delta * synapse->weight;
            synapse->weight -= learnValue * segment->nodes[synapse->targetNodeId].delta * outH;

            netH -= (float)(synapse->border) * BORDER_STEP;
            pos++;
        }
    }
}

/**
 * @brief backpropagateOutput
 * @param segmentHeader
 * @param nodes
 * @param outputNodes
 */
inline void
backpropagateOutput(Segment* segment)
{
    for(ulong outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        __global OutputNode* out = &segment->outputs[outputNodeId];
        __global Node* targetNode = &segment->nodes[out->targetNode];
        const float outW = out->outputWeight;
        const float delta = (outW - out->shouldValue) * outW * (1.0f - outW);
        targetNode->delta = delta;
    }
}

/**
 * @brief correctNewOutputSynapses
 * @param brick
 * @param nodes
 * @param synapseSections
 */
inline void
correctNewOutputSynapses(__global Brick* brick,
                         Segment* segment)
{
    for(uint nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        __global SynapseSection* section = &segment->synapseSections[nodeId];
        if(section->active == 0) {
            continue;
        }

        ushort pos = section->hardening;
        __global Node* sourceNode = &segment->nodes[nodeId];
        float netH = sourceNode->potential;

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            __global Synapse* synapse = &section->synapses[pos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }

            // update weight
            const float delta = segment->nodes[synapse->targetNodeId].delta;
            const bool invert = (delta < 0.0f && synapse->weight < 0.0f)
                                || (delta > 0.0f && synapse->weight > 0.0f);
            if(invert) {
                synapse->weight *= -1.0f;
            }

            netH -= (float)(synapse->border) * BORDER_STEP;
            pos++;
        }
    }
}

inline void
createNewSynapse(__global SynapseSection* section,
                 __global Synapse* synapse,
                 __global Brick* bricks,
                 __global uint* randomValues,
                 __global Node* sourceNode,
                 __global SegmentSettings* synapseMetaData,
                 const float remainingWeight)
{
    float randomMulti = 0.0f;
    float random = 0.0f;
    float doLearn = 0.0f;
    uint targetNodeIdInBrick = 0;
    __global Brick* nodeBrick = NULL;
    uint signRand = 0;

    const float maxWeight = synapseMetaData->maxSynapseWeight;

    // set new weight
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    random = (float)(randomValues[section->randomPos]) / RAND_MAX;
    doLearn = maxWeight * random;
    synapse->weight = (float)(remainingWeight < doLearn) * remainingWeight
                      + (float)(remainingWeight >= doLearn) * doLearn;

    // set activation-border
    synapse->border = (synapse->weight * 255.0f) + 1;

    // update weight with multiplicator
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    randomMulti = (float)(randomValues[section->randomPos]) / RAND_MAX;
    synapse->weight *= randomMulti * (float)(synapseMetaData->multiplicatorRange) + 1.0f;

    // update weight with multiplicator
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    signRand = randomValues[section->randomPos] % 1000;
    synapse->weight *= (float)(1 - (1000.0f * synapseMetaData->signNeg > signRand) * 2);

    // set target node id
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    nodeBrick = &bricks[sourceNode->nodeBrickId];
    const uint targetBrickId = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos];

    __global Brick* targetBrick = &bricks[targetBrickId];
    targetNodeIdInBrick = randomValues[section->randomPos] % targetBrick->numberOfNodes;

    synapse->targetNodeId = (ushort)(targetNodeIdInBrick + targetBrick->nodePos);
    synapse->activeCounter = 1;
}

/**
 * @brief hardenSynapses
 * @param nodes
 * @param synapseSections
 * @param segmentHeader
 */
inline void
hardenSynapses(Segment* segment)
{
    for(uint nodeId = 0;
        nodeId < segment->segmentHeader->nodes.count;
        nodeId++)
    {
        __global Node* sourceNode = &segment->nodes[nodeId];
        __global SynapseSection* section = &segment->synapseSections[nodeId];

        if(section->active == 0) {
            continue;
        }

        if(sourceNode->input > 0.0f) {
            sourceNode->init = 1;
        }

        uint counter = 0;
        ushort pos = 0;
        float netH = sourceNode->potential;

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            __global Synapse* synapse = &section->synapses[pos];
            pos++;

            // process synapse
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }

            netH -= (float)(synapse->border) * BORDER_STEP;
            counter = pos;
        }

        // harden synapse-section
        const bool updateHardening = counter > section->hardening;
        section->hardening = (updateHardening == true) * counter
                             + (updateHardening == false) * section->hardening;
    }
}

/**
 * @brief reduceCoreSynapses
 * @param segmentHeader
 * @param synapseSections
 * @param nodes
 */
inline void
reduceCoreSynapses(Segment* segment)
{
    for(uint sectionId = 0;
        sectionId < segment->segmentHeader->synapseSections.count;
        sectionId++)
    {
        bool upToData = 1;
        __global SynapseSection* section = &segment->synapseSections[sectionId];

        // iterate over all synapses in synapse-section
        uint currentPos = section->hardening;
        for(uint lastPos = section->hardening; lastPos < SYNAPSES_PER_SYNAPSESECTION; lastPos++)
        {
            __global Synapse* synapse = &section->synapses[lastPos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                continue;
            }

            upToData = 0;

            // update dynamic-weight-value of the synapse
            if(segment->nodes[synapse->targetNodeId].active == 0) {
                synapse->activeCounter = -2;
            } else {
                synapse->activeCounter = -2;
            }

            // check for deletion of the single synapse
            if(synapse->activeCounter < 0)
            {
                synapse->weight = 0.0f;
                synapse->targetNodeId = UNINIT_STATE_16;
                synapse->border = 0;
            }
            else
            {
                const Synapse currentSyn = section->synapses[currentPos];
                section->synapses[currentPos] = section->synapses[lastPos];
                section->synapses[lastPos] = currentSyn;
                currentPos++;
            }
        }

        // delete if sections is empty
        if(section->hardening == 0
                && currentPos == 0)
        {
            section->active = 0;
            upToData = 1;
        }
    }
}

/**
 * @brief processInputNodes
 * @param segment
 * @param inputNodes
 */
inline void
processInputNodes(Segment* segment, 
                  __global InputNode* inputs)
{
    for(ulong inputNodeId = 0;
        inputNodeId < segment->segmentHeader->inputs.count;
        inputNodeId++)
    {
        __global const InputNode* inputNode = &inputs[inputNodeId];
        segment->nodes[inputNode->targetNode].input = inputNode->weight;
    }
}

/**
 * @brief processOutputNodes
 * @param segment
 * @param outputNodes
 */
inline void
processOutputNodes(Segment* segment, 
                  __global OutputNode* outputs)
{
    for(ulong outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        __global OutputNode* out = &outputs[outputNodeId];
        __global Node* targetNode = &segment->nodes[out->targetNode];
        const float nodeWeight = targetNode->potential;
        out->outputWeight = 1.0f / (1.0f + exp(-1.0f * nodeWeight));
        segment->outputs[outputNodeId] = outputs[outputNodeId];
    }
}


inline void
synapseProcessing(__global SynapseSection* section,
                  __global Brick* bricks,
                  __global Node* nodes,
                  __global uint* randomValues,
                  __global SegmentSettings* synapseMetaData,
                  __global Node* sourceNode,
                  const float weightIn)
{
    // reinit section if necessary
    if(section->active == 0)
    {
        section->active = 1;
        section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
        section->brickBufferPos = randomValues[section->randomPos] % 1000;
    }

    uint pos = 0;
    float netH = weightIn;
    const float outH = 1.0f / (1.0f + exp(-1.0f * netH));

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        __global Synapse* synapse = &section->synapses[pos];
        const bool createSyn = synapse->targetNodeId == UNINIT_STATE_16
                               && pos >= section->hardening
                               && synapseMetaData->doLearn > 0;
        if(createSyn)
        {
            createNewSynapse(section,
                             synapse,
                             bricks,
                             randomValues,
                             sourceNode,
                             synapseMetaData,
                             netH);
        }

        if(synapse->targetNodeId == UNINIT_STATE_16) {
            break;
        }

        // process synapse
        netH -= (float)(synapse->border) * BORDER_STEP;
        nodes[synapse->targetNodeId].input += outH * synapse->weight;
        synapse->activeCounter += (synapse->activeCounter < 126);

        pos++;
    }
}

/**
 * @brief nodeProcessing
 * @param brick
 * @param nodes
 * @param synapseSections
 * @param bricks
 * @param randomValues
 * @param synapseMetaData
 * @param networkMetaData
 */
inline void
nodeProcessing(__global Brick* brick,
               __global Node* nodes,
               __global SynapseSection* synapseSections,
               __global Brick* bricks,
               __global uint* randomValues,
               __global SegmentSettings* synapseMetaData)
{
    for(uint nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        __global Node* node = &nodes[nodeId];
        const bool initNode = node->init == 0
                              && node->input > 0.0f;
        node->border = (float)(initNode) * node->input * 0.5f
                       + (float)(initNode == false) * node->border;
        node->potential = synapseMetaData->potentialOverflow * node->input;
        node->input = 0.0f;
        node->delta = 0.0f;
    }

    if(brick->isOutputBrick) {
        return;
    }

    for(uint nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        __global Node* node = &nodes[nodeId];
        const bool active = node->potential > node->border;
        if(active)
        {
            synapseProcessing(&synapseSections[nodeId],
                              bricks,
                              nodes,
                              randomValues,
                              synapseMetaData,
                              node,
                              node->potential);
        }

        node->active = active;

    }
}

/**
 * 
 */
Segment parseSegment(__global uchar* persistentData,
                     __global uchar* ephemeralData)
{
    Segment segment;

    segment.segmentHeader = (__global SegmentHeader*)(persistentData + 0);

    segment.synapseSettings = (__global SegmentSettings*)(persistentData + 256);
    // printf("bricks: %d\n" , segment.segmentHeader->bricks.bytePos);
    segment.bricks = (__global Brick*)(persistentData + segment.segmentHeader->bricks.bytePos);
    
    // printf("brickOrder: %d\n" , segment.segmentHeader->brickOrder.bytePos);
    segment.brickOrder = (__global uint*)(persistentData + segment.segmentHeader->brickOrder.bytePos);
    
    // printf("nodes: %d\n" , segment.segmentHeader->nodes.bytePos);
    segment.nodes = (__global Node*)(persistentData + segment.segmentHeader->nodes.bytePos);
    
    // printf("synapseSections: %d\n" , segment.segmentHeader->synapseSections.bytePos);
    segment.synapseSections = (__global SynapseSection*)(persistentData + segment.segmentHeader->synapseSections.bytePos);

    // printf("nodeBuffers: %d\n" , segment.segmentHeader->nodeBuffers.bytePos);
    segment.nodeBuffers = (__global float*)(ephemeralData + segment.segmentHeader->nodeBuffers.bytePos);
    
    // printf("synapseBuffers: %d\n" , segment.segmentHeader->synapseBuffers.bytePos);
    segment.synapseBuffers = (__global SynapseBuffer*)(ephemeralData + segment.segmentHeader->synapseBuffers.bytePos);
    
    // printf("inputs: %d\n" , segment.segmentHeader->inputs.bytePos);
    segment.inputs = (__global InputNode*)(ephemeralData + segment.segmentHeader->inputs.bytePos);

    // printf("outputs: %d\n" , segment.segmentHeader->outputs.bytePos);
    segment.outputs = (__global OutputNode*)(ephemeralData + segment.segmentHeader->outputs.bytePos);

    return segment;
}

/**
 * @brief segmentInputProcessing
 * @param segment
 */
__kernel void
processSegmentInput(__global uchar* persistentData,
                    __global uchar* ephemeralData,
                    __global InputNode* inputs,   
                    __local uchar* localMemory)
{
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);
    const size_t groupId_x = get_group_id(0);


    if(localId_x == 0)
    {
        Segment segment = parseSegment(persistentData, ephemeralData);
        processInputNodes(&segment, inputs);
    }
}

/**
 * @brief segmentOutputProcessing
 * @param segment
 */
__kernel void
processSegmentOutput(__global uchar* persistentData,
                     __global uchar* ephemeralData,
                     __global OutputNode* outputs, 
                     __local uchar* localMemory)
{
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);
    const size_t groupId_x = get_group_id(0);


    if(localId_x == 0)
    {
        Segment segment = parseSegment(persistentData, ephemeralData);
        processOutputNodes(&segment, outputs);

        float totalError = 0.0f;

        for(ulong outputNodeId = 0;
            outputNodeId < segment.segmentHeader->outputs.count;
            outputNodeId++)
        {
            __global OutputNode* out = &segment.outputs[outputNodeId];
            const float diff = (out->shouldValue - out->outputWeight);
            totalError += 0.5f * (diff * diff);
        }

        printf("######################### totalError: %f\n", totalError);
    }
}

/**
 * @brief segmentReduceSynapses
 * @param segment
 */
__kernel void
reduceSegmentSynapses(__global uchar* persistentData,
                      __global uchar* ephemeralData,     
                      __local uchar* localMemory)
{
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);
    const size_t groupId_x = get_group_id(0);


    if(localId_x == 0)
    {
        Segment segment = parseSegment(persistentData, ephemeralData);
        reduceCoreSynapses(&segment);
    }
}

/**
 * @brief segmentBackpropagation
 * @param segment
 */
__kernel void
rewightSegment(__global uchar* persistentData,
               __global uchar* ephemeralData,     
               __local uchar* localMemory)
{
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);
    const size_t groupId_x = get_group_id(0);

    if(localId_x == 0)
    {
        Segment segment = parseSegment(persistentData, ephemeralData);
        const uint numberOfBricks = segment.segmentHeader->bricks.count;

        for(int pos = numberOfBricks - 1; pos >= 0; pos--)
        {
            const uint brickId = segment.brickOrder[pos];
            __global Brick* brick = &segment.bricks[brickId];
            if(brick->isOutputBrick) {
                correctNewOutputSynapses(brick, &segment);
                //printf("poi\n");
            }
        }

       backpropagateOutput(&segment);

        for(int pos = numberOfBricks - 1; pos >= 0; pos--)
        {
            const uint brickId = segment.brickOrder[pos];
            __global Brick* brick = &segment.bricks[brickId];
            backpropagateNodes(brick, &segment);
        }
    }
}

/**
 * @brief segmentHardeing
 * @param segment
 */
__kernel void
hardenSegment(__global uchar* persistentData,
              __global uchar* ephemeralData,  
              __local uchar* localMemory)
{
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);
    const size_t groupId_x = get_group_id(0);


    if(localId_x == 0)
    {
        Segment segment = parseSegment(persistentData, ephemeralData);
        hardenSynapses(&segment);
    }
}

/**
 * @brief segmentNodeProcessing
 * @param segment
 */
__kernel void
prcessSegmentNodes(__global uchar* persistentData,
                   __global uchar* ephemeralData,    
                   __global uint* randomValues, 
                   __local uchar* localMemory)
{
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);
    const int localSize_x = get_local_size(0);
    const size_t groupId_x = get_group_id(0);


    if(localId_x == 0)
    {
        Segment segment = parseSegment(persistentData, ephemeralData);
        const uint numberOfBricks = segment.segmentHeader->bricks.count;

        for(uint pos = 0; pos < numberOfBricks; pos++)
        {
            const uint brickId = segment.brickOrder[pos];
            __global Brick* brick = &segment.bricks[brickId];
            nodeProcessing(brick,
                           segment.nodes,
                           segment.synapseSections,
                           segment.bricks,
                           randomValues,
                           segment.synapseSettings);
        }
    }
}

//==================================================================================================
