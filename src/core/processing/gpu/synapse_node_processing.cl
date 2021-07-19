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
    uchar updated;
    uchar padding1[2];
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
    ulong segmentDataSize;

    // synapse-segment
    SegmentHeaderEntry settings;
    SegmentHeaderEntry bricks;
    SegmentHeaderEntry brickOrder;
    SegmentHeaderEntry synapseSections;
    SegmentHeaderEntry nodes;
    SegmentHeaderEntry inputs;
    SegmentHeaderEntry outputs;

    uchar padding[136];

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
                   Segment* segment,
                   __local uchar* localBuffer)
{
    __local SynapseSection* section = (__local SynapseSection*)localBuffer;

    for(uint nodeId = brick->nodePos + get_local_id(0);
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId += get_local_size(0))
    {
        section[0] = segment->synapseSections[nodeId];
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
            __local Synapse* synapse = &section->synapses[pos];
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

        segment->synapseSections[nodeId] = section[0];
    }
}

/**
 * @brief backpropagateOutput
 * @param segmentHeader
 * @param nodes
 * @param outputNodes
 */
inline void
backpropagateOutput(Segment* segment,
                    __global OutputNode* outputs,
                    __local uchar* localBuffer)
{
    __local OutputNode* out = (__local OutputNode*)localBuffer;

    for(ulong outputNodeId = get_local_id(0);
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId += get_local_size(0))
    {
        out[0] = outputs[outputNodeId];
        __global Node* targetNode = &segment->nodes[out->targetNode];
        const float outW = out->outputWeight;
        const float delta = (outW - out->shouldValue) * outW * (1.0f - outW);
        targetNode->delta = delta;
        outputs[outputNodeId] = out[0];
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
                         Segment* segment,
                         __local uchar* localBuffer)
{
    __local SynapseSection* section = (__local SynapseSection*)localBuffer;

    for(uint nodeId = brick->nodePos + get_local_id(0);
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId += get_local_size(0))
    {
        section[0] = segment->synapseSections[nodeId];
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
            __local Synapse* synapse = &section->synapses[pos];
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

        segment->synapseSections[nodeId] = section[0];
    }
}

inline void
createNewSynapse(__local SynapseSection* section,
                 __local Synapse* synapse,
                 __global Brick* bricks,
                 __global uint* randomValues,
                 __global Node* sourceNode,
                 __local SegmentSettings* segmentSettings,
                 const float remainingWeight)
{
    float randomMulti = 0.0f;
    float random = 0.0f;
    float doLearn = 0.0f;
    uint targetNodeIdInBrick = 0;
    __global Brick* nodeBrick = 0;
    uint signRand = 0;

    const float maxWeight = segmentSettings->maxSynapseWeight;

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
    synapse->weight *= randomMulti * (float)(segmentSettings->multiplicatorRange) + 1.0f;

    // update weight with multiplicator
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    signRand = randomValues[section->randomPos] % 1000;
    synapse->weight *= (float)(1 - (1000.0f * segmentSettings->signNeg > signRand) * 2);

    // set target node id
    section->randomPos = (section->randomPos + 1) % NUMBER_OF_RAND_VALUES;
    nodeBrick = &bricks[sourceNode->nodeBrickId];
    const uint targetBrickId = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos];

    __global Brick* targetBrick = &bricks[targetBrickId];
    targetNodeIdInBrick = randomValues[section->randomPos] % targetBrick->numberOfNodes;

    synapse->targetNodeId = (ushort)(targetNodeIdInBrick + targetBrick->nodePos);
    synapse->activeCounter = 1;
    section->updated = 1;
}

/**
 * @brief hardenSynapses
 * @param nodes
 * @param synapseSections
 * @param segmentHeader
 */
inline void
hardenSynapses(Segment* segment,
               __local uchar* localBuffer)
{
    __local SynapseSection* section = (__local SynapseSection*)localBuffer;
    for(uint nodeId = get_local_id(0);
        nodeId < segment->segmentHeader->nodes.count;
        nodeId += get_local_size(0))
    {
        __global Node* sourceNode = &segment->nodes[nodeId];
        section[0] = segment->synapseSections[nodeId];

        if(section->active == 0
                || section->updated == 0) 
        {
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
            __local Synapse* synapse = &section->synapses[pos];
            pos++;

            // process synapse
            if(synapse->targetNodeId != UNINIT_STATE_16) 
            {
                netH -= (float)(synapse->border) * BORDER_STEP;
                counter = pos;
            }
        }

        // harden synapse-section
        const bool updateHardening = counter > section->hardening;
        section->hardening = (updateHardening == true) * counter
                             + (updateHardening == false) * section->hardening;

        segment->synapseSections[nodeId] = section[0];
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
void
processInputNodes(Segment* segment, 
                  __global InputNode* inputs,
                  __local uchar* localBuffer)
{
    __local InputNode* inputNode = (__local InputNode*)localBuffer;

    for(ulong inputNodeId = get_local_id(0);
        inputNodeId < segment->segmentHeader->inputs.count;
        inputNodeId += get_local_size(0))
    {
        inputNode[0] = inputs[inputNodeId];
        segment->nodes[inputNode->targetNode].input = inputNode->weight;
    }
}

/**
 * @brief processOutputNodes
 * @param segment
 * @param outputNodes
 */
void
processOutputNodes(Segment* segment, 
                   __global OutputNode* outputs,
                   __local uchar* localBuffer)
{
    __local OutputNode* out = (__local OutputNode*)localBuffer;

    for(ulong outputNodeId = get_local_id(0);
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId += get_local_size(0))
    {
        out[0] = outputs[outputNodeId];
        __global Node* targetNode = &segment->nodes[out->targetNode];
        const float nodeWeight = targetNode->potential;
        out->outputWeight = 1.0f / (1.0f + exp(-1.0f * nodeWeight));
        outputs[outputNodeId] = out[0];
    }
}


inline void
synapseProcessing(__local SynapseSection* section,
                  __global Brick* bricks,
                  __global Node* nodes,
                  __global uint* randomValues,
                  __local SegmentSettings* segmentSettings,
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
    section->updated = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        __local Synapse* synapse = &section->synapses[pos];
        const bool createSyn = synapse->targetNodeId == UNINIT_STATE_16
                               && pos >= section->hardening
                               && segmentSettings->doLearn > 0;
        if(createSyn)
        {
            createNewSynapse(section,
                             synapse,
                             bricks,
                             randomValues,
                             sourceNode,
                             segmentSettings,
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
 * @param segmentSettings
 * @param networkMetaData
 */
void
nodeProcessing(__global Brick* brick,
               __global Node* nodes,
               __global SynapseSection* synapseSections,
               __global Brick* bricks,
               __global uint* randomValues,
               __local SegmentSettings* segmentSettings,
               __local uchar* localBuffer)
{
    __local Node* node = (__local Node*)localBuffer;

    for(uint nodeId = brick->nodePos + get_local_id(0);
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId += get_local_size(0))
    {
        node[0] = nodes[nodeId];

        const bool initNode = node->init == 0
                              && node->input > 0.0f;
        node->border = (float)(initNode) * node->input * 0.5f
                       + (float)(initNode == false) * node->border;
        node->potential = segmentSettings->potentialOverflow * node->input;
        node->input = 0.0f;
        node->delta = 0.0f;

        nodes[nodeId] = node[0];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    if(brick->isOutputBrick) {
        return;
    }

    //__local SynapseSection* localSynapseBuffer = (__local SynapseSection*)localBuffer;
    __local SynapseSection* localSynapseBuffer = (__local SynapseSection*)localBuffer;
    for(uint nodeId = brick->nodePos + get_local_id(0);
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId += get_local_size(0))
    {
        __global Node* node = &nodes[nodeId];
        localSynapseBuffer[0] = synapseSections[nodeId];

        const bool active = node->potential > node->border;
        if(active)
        {
            synapseProcessing(localSynapseBuffer,
                              bricks,
                              nodes,
                              randomValues,
                              segmentSettings,
                              node,
                              node->potential);
        }
        synapseSections[nodeId] = localSynapseBuffer[0];
        node->active = active;

    }
}

/**
 * 
 */
inline Segment 
parseSegment(__global uchar* segmentData)
{
    Segment segment;

    segment.segmentHeader = (__global SegmentHeader*)(segmentData + 0);

    segment.synapseSettings = (__global SegmentSettings*)(segmentData + 256);
    // printf("bricks: %d\n" , segment.segmentHeader->bricks.bytePos);
    segment.bricks = (__global Brick*)(segmentData + segment.segmentHeader->bricks.bytePos);
    
    // printf("brickOrder: %d\n" , segment.segmentHeader->brickOrder.bytePos);
    segment.brickOrder = (__global uint*)(segmentData + segment.segmentHeader->brickOrder.bytePos);
    
    // printf("nodes: %d\n" , segment.segmentHeader->nodes.bytePos);
    segment.nodes = (__global Node*)(segmentData + segment.segmentHeader->nodes.bytePos);
    
    // printf("synapseSections: %d\n" , segment.segmentHeader->synapseSections.bytePos);
    segment.synapseSections = (__global SynapseSection*)(segmentData + segment.segmentHeader->synapseSections.bytePos);

    // printf("inputs: %d\n" , segment.segmentHeader->inputs.bytePos);
    segment.inputs = (__global InputNode*)(segmentData + segment.segmentHeader->inputs.bytePos);

    // printf("outputs: %d\n" , segment.segmentHeader->outputs.bytePos);
    segment.outputs = (__global OutputNode*)(segmentData + segment.segmentHeader->outputs.bytePos);

    return segment;
}

/**
 * @brief segmentBackpropagation
 * @param segment
 */
void
rewightSegment(Segment segment,
               __global OutputNode* outputs,
               __local uchar* localBuffer)
{
    const uint numberOfBricks = segment.segmentHeader->bricks.count;

    for(int pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        const uint brickId = segment.brickOrder[pos];
        __global Brick* brick = &segment.bricks[brickId];
        if(brick->isOutputBrick) {
            correctNewOutputSynapses(brick, &segment, localBuffer);
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    backpropagateOutput(&segment, outputs, localBuffer);
    barrier(CLK_LOCAL_MEM_FENCE);

    for(int pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        const uint brickId = segment.brickOrder[pos];
        __global Brick* brick = &segment.bricks[brickId];
        backpropagateNodes(brick, &segment, localBuffer);
        barrier(CLK_LOCAL_MEM_FENCE);
    }
}

/**
 * @brief segmentNodeProcessing
 * @param segment
 */
void
prcessSegmentNodes(Segment segment,
                   __global uint* randomValues,
                   __local SegmentSettings* localSegmentSettings, 
                   __local uchar* localBuffer)
{
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
                       localSegmentSettings,
                       localBuffer);
        barrier(CLK_LOCAL_MEM_FENCE);
    }
}

//==================================================================================================

__kernel void
learn(__global uchar* segmentData,
      __global InputNode* inputs,   
      __global OutputNode* outputs, 
      __global uint* randomValues, 
      __local uchar* localMemory)
{
    __local SegmentHeader* localSegmentHeader = (__local SegmentHeader*)&localMemory[0];
    __local SegmentSettings* localSegmentSettings = (__local SegmentSettings*)&localMemory[256];
    __local uchar* localBuffer = (__local uchar*)&localMemory[512 + (get_local_id(0) * 512)];

    Segment segment = parseSegment(segmentData);
    segment.synapseSettings->doLearn = 1;

    if(get_local_id(0) == 0) 
    {
        localSegmentHeader[0] = segment.segmentHeader[0];
        localSegmentSettings[0] = segment.synapseSettings[0];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    processInputNodes(&segment, inputs, localBuffer);
    barrier(CLK_LOCAL_MEM_FENCE);

    prcessSegmentNodes(segment, randomValues, localSegmentSettings, localBuffer);
    barrier(CLK_LOCAL_MEM_FENCE);

    processOutputNodes(&segment, outputs, localBuffer);
    barrier(CLK_LOCAL_MEM_FENCE);

    if(get_local_id(0) == 0) 
    {
        float totalError = 0.0f;

        for(ulong outputNodeId = 0;
            outputNodeId < segment.segmentHeader->outputs.count;
            outputNodeId++)
        {
            __global OutputNode* out = &outputs[outputNodeId];
            const float diff = (out->shouldValue - out->outputWeight);
            totalError += 0.5f * (diff * diff);
        }

        printf("######################### totalError: %f\n", totalError);
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    rewightSegment(segment, outputs, localBuffer);
    barrier(CLK_LOCAL_MEM_FENCE);

    hardenSynapses(&segment, localBuffer);
}

__kernel void
execute(__global uchar* segmentData,
        __global InputNode* inputs,   
        __global OutputNode* outputs,  
        __global uint* randomValues, 
        __local uchar* localMemory)
{
    __local SegmentHeader* localSegmentHeader = (__local SegmentHeader*)&localMemory[0];
    __local SegmentSettings* localSegmentSettings = (__local SegmentSettings*)&localMemory[256];
    __local uchar* localBuffer = (__local uchar*)&localMemory[512 + (get_local_id(0) * 512)];

    Segment segment = parseSegment(segmentData);
    segment.synapseSettings->doLearn = 1;
    if(get_local_id(0) == 0) 
    {
        localSegmentHeader[0] = segment.segmentHeader[0];
        localSegmentSettings[0] = segment.synapseSettings[0];
    }
    barrier(CLK_LOCAL_MEM_FENCE);

    processInputNodes(&segment, inputs, localBuffer);
    barrier(CLK_LOCAL_MEM_FENCE);

    prcessSegmentNodes(segment, randomValues, localSegmentSettings, localBuffer);
    barrier(CLK_LOCAL_MEM_FENCE);

    processOutputNodes(&segment, outputs, localBuffer);
}
