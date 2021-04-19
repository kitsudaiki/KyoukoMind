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

/**
 * @brief synapseProcessing
 * @param sectionPos
 * @param weight
 * @param hardening
 */
inline void
synapseProcessing(__local SynapseSection* section,
                  __global Node* nodes,
                  __global Brick* bricks,
                  __global float* nodeBuffers,
                  __global SynapseBuffer* synapseBuffers,
                  __local SynapseSegmentMeta* segmentMeta,
                  __global uint* randomValues,
                  __local SynapseMetaData* synapseMetaData,
                  __global NetworkMetaData* networkMetaData,
                  const uint nodeId,
                  const float weightIn,
                  const uint layer)

{
    uint pos = 0;
    uint counter = 0;
    float weight = weightIn;
    bool processed = false;
    const float maxWeight = synapseMetaData->maxSynapseWeight;
    __global Node* node = &nodes[nodeId];

    // reinit section if necessary
    if(section->active == 0)
    {
        section->active = 1;
        section->randomPos = (section->randomPos + 1) % segmentMeta->numberOfRandomValues;
        section->brickBufferPos = randomValues[section->randomPos] % 1000;
    }

    // iterate over all synapses in the section and update the target-nodes
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && weight > 0.0f)
    {
        __local Synapse* synapse = &section->synapses[pos];

        // create new synapse
        if(synapse->targetNodeId == UNINIT_STATE_16
                && pos >= section->hardening
                && networkMetaData->doLearn > 0)
        {
            // set new weight
            section->randomPos = (section->randomPos + 1) % segmentMeta->numberOfRandomValues;
            const float random = static_cast<float>(randomValues[section->randomPos]) / RAND_MAX;
            const float tooLearn = maxWeight * random;
            synapse->weight = static_cast<float>(weight < tooLearn) * weight
                                    + static_cast<float>(weight >= tooLearn) * tooLearn;

            // get random node-id as target
            section->randomPos = (section->randomPos + 1) % segmentMeta->numberOfRandomValues;
            const uint targetNodeIdInBrick = randomValues[section->randomPos] % segmentMeta->numberOfNodesPerBrick;
            Brick* nodeBrick = &bricks[node->nodeBrickId];
            const uint nodeOffset = nodeBrick->possibleTargetNodeBrickIds[section->brickBufferPos]
                                        * segmentMeta->numberOfNodesPerBrick;
            synapse->targetNodeId = static_cast<ushort>(targetNodeIdInBrick + nodeOffset);

            // set sign
            section->randomPos = (section->randomPos + 1) % segmentMeta->numberOfRandomValues;
            const uint signRand = randomValues[section->randomPos] % 1000;
            const float signNeg = synapseMetaData->signNeg;
            synapse->sign = 1 - (1000.0f * signNeg > signRand) * 2;

            section->randomPos = (section->randomPos + 1) % segmentMeta->numberOfRandomValues;
            synapse->multiplicator = static_cast<char>((randomValues[section->randomPos] % synapseMetaData->multiplicatorRange) + 1);
        }

        pos++;

        // process synapse
        if(synapse->targetNodeId != UNINIT_STATE_16)
        {
            // 0 because only one thread at the moment
            const ulong nodeBufferPosition = (0 * segmentMeta->numberOfNodes) + synapse->targetNodeId;
            const float synapseWeight = synapse->weight;
            const float shareWeight = static_cast<float>(weight > synapseWeight) * synapseWeight
                                      + static_cast<float>(weight <= synapseWeight) * weight;

            nodeBuffers[nodeBufferPosition] += (shareWeight * static_cast<float>(synapse->sign) * static_cast<float>(synapse->multiplicator));

            weight -= shareWeight;
            counter = pos;
            processed = true;
        }
    }

    // harden synapse-section
    if(networkMetaData->lerningValue > 0.0f)
    {
        if(counter > section->hardening) {
            section->hardening = counter;
        }
    }

    // go to next section
    if(weight > 1.0f
            && processed)
    {
        uint nextLayer = layer + 1;
        nextLayer = (nextLayer > 7) * 7  + (nextLayer <= 7) * nextLayer;
        const uint pos = (node->targetSectionId + nextLayer * 10000 + nextLayer) % segmentMeta->numberOfSynapseSections;
        SynapseBuffer* synapseBuffer = &synapseBuffers[pos];
        synapseBuffer->buffer[nextLayer].weigth = weight;
        synapseBuffer->buffer[nextLayer].nodeId = nodeId;
        synapseBuffer->process = 1;
    }
}

/**
 * @brief updating
 * @param sectionPos
 */
inline bool
updating(__local SynapseSection* section,
         __global Node* nodes,
         __local SynapseMetaData* synapseMetaData)
{
    bool upToData = 1;

    // iterate over all synapses in synapse-section
    uint currentPos = section->hardening;
    for(uint lastPos = section->hardening; lastPos < SYNAPSES_PER_SYNAPSESECTION; lastPos++)
    {
        __local Synapse* synapse = &section->synapses[lastPos];

        if(synapse->targetNodeId == UNINIT_STATE_16) {
            continue;
        }

        upToData = 0;

        // update dynamic-weight-value of the synapse
        if(nodes[synapse->targetNodeId].active == 0) {
            synapse->weight = synapse->weight * 0.0f;
        } else {
            synapse->weight = synapse->weight * 0.0f;
        }

        // check for deletion of the single synapse
        if(synapse->weight < synapseMetaData->synapseDeleteBorder)
        {
            synapse->weight = 0.0f;
            synapse->targetNodeId = UNINIT_STATE_16;
            synapse->sign = 1;
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

    return upToData;
}

/**
 * @brief synapse_processing
 */
__kernel void
synapse_processing(__global SynapseSegmentMeta* segmentMeta,
                   __global SynapseBuffer* synapseBuffers,
                   __global SynapseSection* synapseSections,
                   __global Node* nodes,
                   __global Brick* bricks,
                   __global float* nodeBuffers,
                   __global uint* randomValues,
                   __global SynapseMetaData* synapseMetaData,
                   __global NetworkMetaData* networkMetaData,         
                   __local uchar* localMemory)
{
        // prepare coordinates
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);

    // prepare shared memory
    __local OutputSegmentMeta* localSegmentMeta = (__local OutputSegmentMeta*)&localMemory[0];
    localSegmentMeta[0] = segmentMeta[0];
    __local SynapseMetaData* localSynapseMetaData = (__local SynapseMetaData*)&localMemory[256];
    localSynapseMetaData[0] = synapseMetaData[0];


    __local SynapseSection* localSynapseSections = (__local SynapseSection*)&localMemory[512];

    //----------------------------------------------------------------------------------------------
    for(ulong i = globalId_x; i < segmentMeta->numberOfSynapseSections; i = i + globalSize_x)
    {
        localSynapseSections[localId_x] = synapseSections[i];

        __global SynapseBuffer* synapseBuffer = &synapseBuffers[i];

        if(synapseBuffer->process == 0)
        {
            if(synapseBuffer->upToDate == 0) 
            {
                synapseBuffer->upToDate = updating(&localSynapseSections[i],
                                                   nodes,
                                                   localSynapseMetaData);
            }
            continue;
        }

        for(uchar layer = 0; layer < 8; layer++)
        {
            __global SynapseBufferEntry* entry = &synapseBuffer->buffer[layer];

            if(entry->weigth > 5.0f)
            {
                synapseProcessing(&localSynapseSections[i],
                                  nodes,
                                  bricks,
                                  nodeBuffers,
                                  synapseBuffers,
                                  localSegmentMeta,
                                  randomValues,
                                  localSynapseMetaData,
                                  networkMetaData,
                                  entry->nodeId,
                                  entry->weigth,
                                  layer);
                synapseBuffer->upToDate = 0;
            }

            entry->weigth = 0.0f;
            entry->nodeId = UNINIT_STATE_32;
        }

        synapseSections[i] = localSynapseSections[localId_x];
    }
}

/**
 * @brief node_processing
 */
__kernel void
node_processing(__global Node* nodes,
                __global float* nodeBuffers,
                __global InputNode* inputNodes,
                __global SynapseBuffer* synapseBuffers,
                __global SynapseSegmentMeta* segmentMeta,
                __global SynapseMetaData* synapseMetaData,
                __global float* outputInputs,         
                __local uchar* localMemory)
{
    // prepare coordinates
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);

    // prepare shared memory
    __local OutputSegmentMeta* localSegmentMeta = (__local OutputSegmentMeta*)&localMemory[0];
    localSegmentMeta[0] = segmentMeta[0];
    __local OutputMetaData* localOutputMetaData = (__local OutputMetaData*)&localMemory[256];
    localOutputMetaData[0] = outputMetaData[0];

    //--------------------------------------------------------------------------------------------------------------

    __local Node* localNodes1 = (__local Node*)&localMemory[512];

    for(ulong i = globalId_x; i < segmentMeta->numberOfNodes; i = i + globalSize_x)
    {
        localNodes1[localId_x] = nodes[i];

        for(uint pos = 0; pos < 127; pos++)
        {
            const ulong nodeBufferPosition = (pos * (segmentMeta->numberOfNodes)) + i;
            localNodes1[localId_x].currentState += nodeBuffers[nodeBufferPosition];
            nodeBuffers[nodeBufferPosition] = 0.0f;
        }

        nodes[i] = localNodes1[localId_x] ;
    }

    barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

    //--------------------------------------------------------------------------------------------------------------

    for(ulong i = globalId_x; i < segmentMeta->numberOfInputs; i = i + globalSize_x)
    {
        nodes[inputNodes[i].targetNode].potential = inputNodes[i].weight;
    }

    barrier(CLK_LOCAL_MEM_FENCE | CLK_GLOBAL_MEM_FENCE);

    //--------------------------------------------------------------------------------------------------------------

    __local Node* localNodes2 = (__local Node*)&localMemory[512];

    for(ulong i = globalId_x; i < segmentMeta->numberOfNodes; i = i + globalSize_x)
    {
        localNodes2[localId_x] = nodes[i];
        __local Node* node = &localNodes2[localId_x];

        if(node->border > 0.0f)
        {
            // check if active
            const bool reset = node->border < node->currentState
                               && node->refractionTime == 0;
            if(reset)
            {
                node->potential = synapseMetaData->actionPotential +
                                  synapseMetaData->potentialOverflow * node->currentState;
                node->refractionTime = synapseMetaData->refractionTime;
            }

            // set to 255.0f, if value is too high
            const float cur = node->currentState;
            node->currentState = static_cast<float>(cur < 0.0f) * 0.0f + static_cast<float>(cur >= 0.0f) * cur;

            synapseBuffers[i].buffer[0].weigth = node->potential;
            synapseBuffers[i].buffer[0].nodeId = i;
            synapseBuffers[i].process = node->potential > 5.0f;

            // post-steps
            node->refractionTime = node->refractionTime >> 1;

            // set to 0.0f, if value is negative
            const float newCur = node->currentState;
            node->currentState = static_cast<float>(newCur < 0.0f) * 0.0f + static_cast<float>(newCur >= 0.0f) * newCur;

            // make cooldown in the node
            node->potential /= synapseMetaData->nodeCooldown;
            node->currentState /= synapseMetaData->nodeCooldown;
        }
        else if(node->border == 0.0f)
        {
            synapseBuffers[i].buffer[0].weigth = node->potential;
            synapseBuffers[i].buffer[0].nodeId = i;
            synapseBuffers[i].process = node->potential > 5.0f;
        }
        else
        {
            const float newCur = node->currentState;
            node->currentState = static_cast<float>(newCur < 0.0f) * 0.0f + static_cast<float>(newCur >= 0.0f) * newCur;
            const float pot = synapseMetaData->potentialOverflow * node->currentState;
            outputInputs[i % segmentMeta->numberOfNodesPerBrick] = pot;
            node->currentState = 0.0f;
        }

        nodes[i] = localNodes2[localId_x] ;
    }
}

//==================================================================================================

/**
 * @brief outputSynapseProcessing
 * @param outputSection
 * @param outSectionPos
 * @return
 */
inline float
outputSynapseProcessing(__local OutputSynapseSection* outputSection,
                        __global float* inputs,
                        __local OutputSegmentMeta* segmentMeta,
                        __global NetworkMetaData* networkMetaData,
                        __local OutputMetaData* outputMetaData)
{
    float outputWeight = 0.0f;

    outputSection->total = 0;

    uint pos = 0;
    while(pos < OUTPUT_SYNAPSES_PER_SECTION)
    {
        __local OutputSynapse* synapse = &outputSection->synapses[pos];

        if(networkMetaData->lerningValue > 0.0f)
        {
            synapse->newOne = 0;
            if(synapse->weight == 0.0f)
            {
                outputSection->synapses[pos] = OutputSynapse();
                pos++;
                continue;
            }
        }

        const uint targetId = synapse->targetId;
        if(targetId != UNINIT_STATE_32)
        {
            assert(targetId < segmentMeta->numberOfInputs);
            synapse->active = inputs[targetId] >= outputMetaData->lowerMatch * synapse->border
                              && inputs[targetId] <= outputMetaData->upperMatch * synapse->border;
            outputWeight += synapse->weight * static_cast<float>(synapse->active);
            outputSection->total += synapse->active;
        }

        pos++;
    }

    return outputWeight;
}

/**
 * @brief learNewOutput
 * @param outputSection
 * @return
 */
inline void
learNewOutput(__local OutputSynapseSection* section,
              __global float* inputs,
              __local OutputSegmentMeta* segmentMeta,
              __global uint* randomValues,
              __global NetworkMetaData* networkMetaData,
              __local OutputMetaData* outputMetaData,
              const uint outputPos)
{
    section->newOnes = 0;
    int toNew = static_cast<int>(outputMetaData->maxConnections)
                    - static_cast<int>(section->total);
    if(toNew <= 0) {
        return;
    }

    uint limiter = 0;
    uint pos = 0;
    while(pos < OUTPUT_SYNAPSES_PER_SECTION)
    {
        __local OutputSynapse* synapse = &section->synapses[pos];

        if(synapse->targetId == UNINIT_STATE_32
                && networkMetaData->doLearn > 0
                && limiter < 10)
        {
            // const uint possibleTargetId = rand() % segment->segmentMeta->numberOfInputs;
            section->randomPos = (section->randomPos + 1) % segmentMeta->numberOfRandomValues;
            uint possibleTargetId = randomValues[section->randomPos] % outputMetaData->inputRange;
            possibleTargetId += outputPos * outputMetaData->inputOffset;
            assert(possibleTargetId <= segmentMeta->numberOfInputs);
            if(inputs[possibleTargetId] > 0.0f)
            {
                synapse->targetId = possibleTargetId;
                synapse->border = inputs[possibleTargetId];
                synapse->weight = 0.0f;
                synapse->newOne = 1;
                synapse->active = 1;
                limiter++;
            }
        }

        if(section->newOnes == static_cast<uint>(toNew)
                && synapse->newOne == 1)
        {
            section->synapses[pos] = OutputSynapse();
        }

        if(synapse->newOne == 1) {
            section->newOnes++;
            section->total++;
        }

        pos++;
    }
}

/**
 * @brief outputSynapseLearn
 * @param outputSection
 * @return
 */
inline void
outputSynapseLearn(__global OutputSynapseSection* outputSection)
{
    uint pos = 0;
    while(pos < OUTPUT_SYNAPSES_PER_SECTION)
    {
        // update target
        __global OutputSynapse* synapse = &outputSection->synapses[pos];
        if(synapse->targetId != UNINIT_STATE_32)
        {
            if(synapse->active > 0)
            {
                synapse->weight += outputSection->diffTotal;
            }
        }

        pos++;
    }
}

/**
 * @brief calculateLearnings
 * @param outputSection
 * @param out
 * @return
 */
inline float
calculateLearnings(__local OutputSynapseSection* outputSection,
                   __global Output* output)
{
    outputSection->diffNew = output->shouldValue - output->outputValue;
    outputSection->diffTotal = output->shouldValue - output->outputValue;

    if(output->shouldValue == 0.0f
            && output->outputValue <= output->shouldValue)
    {
        outputSection->diffNew = 0.0f;
        outputSection->diffTotal = 0.0f;
    }

    if(output->shouldValue > 0.0f
            && output->outputValue >= output->shouldValue)
    {
        outputSection->diffNew = 0.0f;
        outputSection->diffTotal = 0.0f;
    }

    const float totalDiff = fabs(outputSection->diffNew);

    outputSection->diffNew /= static_cast<float>(outputSection->newOnes + 1);
    outputSection->diffTotal /= static_cast<float>(outputSection->total + 1);

    return totalDiff;
}

/**
 * @brief node_processing
 */
__kernel void
output_node_processing(__global OutputSynapseSection* outputSynapseSections,
                       __global float* inputs,
                       __global Output* outputs,
                       __global OutputSegmentMeta* segmentMeta,
                       __global NetworkMetaData* networkMetaData,
                       __global OutputMetaData* outputMetaData,         
                       __local uchar* localMemory)
{
    // prepare coordinates
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);

    // prepare shared memory
    __local OutputSegmentMeta* localSegmentMeta = (__local OutputSegmentMeta*)&localMemory[0];
    localSegmentMeta[0] = segmentMeta[0];
    __local OutputMetaData* localOutputMetaData = (__local OutputMetaData*)&localMemory[256];
    localOutputMetaData[0] = outputMetaData[0];
    __local OutputSynapseSection* localOutputSynapseSections = (__local OutputSynapseSection*)&localMemory[512];

    // process output
    for(ulong o = globalId_x; o < localSegmentMeta->numberOfOutputs; o = o + globalSize_x)
    {
        localOutputSynapseSections[localId_x] = outputSynapseSections[o];

        outputs[o].outputValue = outputSynapseProcessing(&localOutputSynapseSections[localId_x] ,
                                                         inputs,
                                                         localSegmentMeta,
                                                         networkMetaData,
                                                         localOutputMetaData);

        outputSynapseSections[o] = localOutputSynapseSections[localId_x];
    }
}

/**
 * @brief output_learn_step
 * @return
 */
__kernel void
output_learn_step(__global OutputSynapseSection* outputSynapseSections,
                  __global float* inputs,
                  __global Output* outputs,
                  __global OutputSegmentMeta* segmentMeta,
                  __global uint* randomValues,
                  __global NetworkMetaData* networkMetaData,
                  __global OutputMetaData* outputMetaData,         
                  __local uchar* localMemory)
{
    // prepare coordinates
    const size_t globalId_x = get_global_id(0);
    const size_t globalSize_x = get_global_size(0);
    const int localId_x = get_local_id(0);

    // prepare shared memory
    __local OutputSegmentMeta* localSegmentMeta = (__local OutputSegmentMeta*)&localMemory[0];
    localSegmentMeta[0] = segmentMeta[0];
    __local OutputMetaData* localOutputMetaData = (__local OutputMetaData*)&localMemory[256];
    localOutputMetaData[0] = outputMetaData[0];
    __local OutputSynapseSection* localOutputSynapseSections = (__local OutputSynapseSection*)&localMemory[512];

    for(ulong o = globalId_x; o < localSegmentMeta->numberOfOutputs; o = o + globalSize_x)
    {
        localOutputSynapseSections[localId_x] = outputSynapseSections[o];

        learNewOutput(&localOutputSynapseSections[localId_x],
                      inputs,
                      segmentMeta,
                      randomValues,
                      networkMetaData,
                      outputMetaData,
                      o);
        calculateLearnings(&localOutputSynapseSections[localId_x], &outputs[o]);
        if(outputSynapseSections[o].diffTotal != 0.0f)
        {
            outputSynapseLearn(&outputSynapseSections[o]);
            outputs[o].outputValue = outputSynapseProcessing(&localOutputSynapseSections[localId_x],
                                                             inputs,
                                                             localSegmentMeta,
                                                             networkMetaData,
                                                             localOutputMetaData);
        }

        outputSynapseSections[o] = localOutputSynapseSections[localId_x];
    }
}

//==================================================================================================
