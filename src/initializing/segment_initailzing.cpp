/**
 * @file        segment_initailzing.cpp
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

#include "segment_initailzing.h"

#include <libKitsunemimiAiParser/ai_parser_input.h>
#include <libKitsunemimiAiCommon/metadata.h>

/**
 * @brief Segment::initOutputSegment
 * @param numberOfOutputs
 * @param numberOfRandValues
 * @return
 */
OutputSegment*
initOutputSegment(const uint32_t numberOfOutputs,
                  const uint32_t numberOfInputs,
                  const uint32_t numberOfRandValues)
{
    OutputSegment* newSegment = new OutputSegment();

    // get total buffer-size for the new segment
    uint32_t totalSize = 0;
    totalSize += 1 * sizeof(OutputSegmentMeta);
    totalSize += 1 * sizeof(Kitsunemimi::Ai::OutputMetaData);
    totalSize += numberOfOutputs * sizeof(OutputSynapseSection);
    totalSize += numberOfOutputs * sizeof(Output);
    totalSize += numberOfInputs * sizeof(float);

    // allocate memory
    const uint32_t numberOfBlocks = (totalSize / 4096) + 1;
    Kitsunemimi::allocateBlocks_DataBuffer(newSegment->buffer, numberOfBlocks);

    uint8_t* data = static_cast<uint8_t*>(newSegment->buffer.data);
    uint32_t bufferPos = 0;

    // init segment meta-data
    newSegment->segmentMeta = reinterpret_cast<OutputSegmentMeta*>(data + bufferPos);
    bufferPos += 1 * sizeof(OutputSegmentMeta);
    newSegment->segmentMeta[0] = OutputSegmentMeta();
    newSegment->segmentMeta->segmentType = OUTPUT_SEGMENT;
    newSegment->segmentMeta->numberOfOutputs = numberOfOutputs;
    newSegment->segmentMeta->numberOfInputs = numberOfInputs;

    // init global values
    newSegment->outputMetaData = reinterpret_cast<Kitsunemimi::Ai::OutputMetaData*>(data + bufferPos);
    bufferPos += 1 * sizeof(Kitsunemimi::Ai::OutputMetaData);
    newSegment->outputMetaData[0] = Kitsunemimi::Ai::OutputMetaData();

    // init output-sections
    newSegment->outputSynapseSections = reinterpret_cast<OutputSynapseSection*>(data + bufferPos);
    bufferPos += numberOfOutputs * sizeof(OutputSynapseSection);
    for(uint32_t i = 0; i < numberOfOutputs; i++)
    {
        newSegment->outputSynapseSections[i] = OutputSynapseSection();
        newSegment->outputSynapseSections[i].randomPos = static_cast<uint32_t>(rand()) % numberOfRandValues;
    }

    // init outputs
    newSegment->outputs = reinterpret_cast<Output*>(data + bufferPos);
    bufferPos += numberOfOutputs * sizeof(Output);
    for(uint32_t i = 0; i < numberOfOutputs; i++) {
        newSegment->outputs[i] = Output();
    }

    // init input
    newSegment->inputs = reinterpret_cast<OutputInput*>(data + bufferPos);
    bufferPos += numberOfInputs * sizeof(float);
    for(uint32_t i = 0; i < numberOfInputs; i++) {
        newSegment->inputs[i] = OutputInput();
    }

    return newSegment;
}

/**
 * @brief Segment::initSynapseSegment
 * @param numberOfNodeBricks
 * @param numberOfNodes
 * @param numberOfSynapseSections
 * @param numberOfRandValues
 * @return
 */
CoreSegment*
initSynapseSegment(const uint32_t numberOfNodeBricks,
                   const uint32_t numberOfNodes,
                   const uint64_t numberOfSynapseSections,
                   const uint32_t numberOfInputs,
                   const uint32_t numberOfRandValues)
{
    CoreSegment* newSegment = new CoreSegment();

    // get total buffer-size for the new segment
    uint32_t totalSize = 0;
    totalSize += 1 * sizeof(CoreSegmentMeta);
    totalSize += 1 * sizeof(Kitsunemimi::Ai::CoreMetaData);
    totalSize += numberOfNodeBricks * sizeof(Brick);
    totalSize += numberOfNodes * sizeof(Node);
    totalSize += numberOfNodes * 127 * sizeof(float);
    totalSize += numberOfSynapseSections * sizeof(SynapseSection);
    totalSize += numberOfSynapseSections * sizeof(SynapseBuffer);
    totalSize += numberOfInputs * sizeof(InputNode);

    // allocate memory
    const uint32_t numberOfBlocks = (totalSize / 4096) + 1;
    Kitsunemimi::allocateBlocks_DataBuffer(newSegment->buffer, numberOfBlocks);

    uint8_t* data = static_cast<uint8_t*>(newSegment->buffer.data);
    uint32_t bufferPos = 0;

    // init segment meta-data
    newSegment->segmentMeta = reinterpret_cast<CoreSegmentMeta*>(data + bufferPos);
    bufferPos += 1 * sizeof(CoreSegmentMeta);
    newSegment->segmentMeta[0] = CoreSegmentMeta();
    newSegment->segmentMeta->segmentType = CORE_SEGMENT;
    newSegment->segmentMeta->numberOfNodeBricks = numberOfNodeBricks;
    newSegment->segmentMeta->numberOfNodesPerBrick = numberOfNodes / numberOfNodeBricks;
    newSegment->segmentMeta->numberOfSynapseSections = numberOfSynapseSections;
    newSegment->segmentMeta->numberOfInputs = numberOfInputs;
    newSegment->segmentMeta->numberOfNodes = numberOfNodes;

    // init global values
    newSegment->synapseMetaData = reinterpret_cast<Kitsunemimi::Ai::CoreMetaData*>(data + bufferPos);
    bufferPos += 1 * sizeof(Kitsunemimi::Ai::CoreMetaData);
    newSegment->synapseMetaData[0] = Kitsunemimi::Ai::CoreMetaData();

    // init node-bricks
    newSegment->nodeBricks = reinterpret_cast<Brick*>(data + bufferPos);
    bufferPos += numberOfNodeBricks * sizeof(Brick);
    for(uint32_t i = 0; i < numberOfNodeBricks; i++) {
        newSegment->nodeBricks[i] = Brick();
    }

    // init nodes
    newSegment->nodes = reinterpret_cast<Node*>(data + bufferPos);
    bufferPos += numberOfNodes * sizeof(Node);
    for(uint32_t i = 0; i < numberOfNodes; i++) {
        newSegment->nodes[i] = Node();
    }

    // init node-buffer
    newSegment->nodeBuffers = reinterpret_cast<float*>(data + bufferPos);
    bufferPos += numberOfNodes * 127 * sizeof(float);
    for(uint32_t i = 0; i < numberOfNodes * 127; i++) {
        newSegment->nodeBuffers[i] = 0.0f;
    }

    // init synapse sections
    newSegment->synapseSections = reinterpret_cast<SynapseSection*>(data + bufferPos);
    bufferPos += numberOfSynapseSections * sizeof(SynapseSection);
    for(uint32_t i = 0; i < numberOfSynapseSections; i++)
    {
        newSegment->synapseSections[i] = SynapseSection();
        newSegment->synapseSections[i].randomPos = static_cast<uint32_t>(rand()) % numberOfRandValues;
    }

    // init synapse buffer
    newSegment->synapseBuffers = reinterpret_cast<SynapseBuffer*>(data + bufferPos);
    bufferPos += numberOfSynapseSections * sizeof(SynapseBuffer);
    for(uint32_t i = 0; i < numberOfSynapseSections; i++) {
        newSegment->synapseBuffers[i] = SynapseBuffer();
    }

    // init input
    newSegment->inputNodes = reinterpret_cast<InputNode*>(data + bufferPos);
    bufferPos += numberOfInputs * sizeof(InputNode);
    for(uint32_t i = 0; i < numberOfInputs; i++) {
        newSegment->inputNodes[i] = InputNode();
    }

    return newSegment;
}

/**
 * @brief initLayer
 * @param segment
 * @return
 */
bool
initLayer(CoreSegment &segment,
          Kitsunemimi::Ai::InitMeataData* initMetaData)
{
    // init layer-buffer
    for(uint32_t i = 0; i < initMetaData->layer + 1; i++) {
        segment.layer.push_back(std::vector<Brick*>());
    }

    for(uint32_t i = 0; i < segment.segmentMeta->numberOfNodeBricks; i++)
    {
        Brick* brickPtr = &segment.nodeBricks[i];

        if(brickPtr->isInputBrick)
        {
            brickPtr->layerId = 0;
            segment.layer[brickPtr->layerId].push_back(brickPtr);
        }
        else if(brickPtr->isOutputBrick)
        {
            brickPtr->layerId = initMetaData->layer;
            segment.layer[brickPtr->layerId].push_back(brickPtr);
        }
        else if(brickPtr->nodeBrickId != UNINIT_STATE_32)
        {
            if(initMetaData->layer == 1) {
                brickPtr->layerId = 1;
            } else {
                brickPtr->layerId = (brickPtr->brickPos.x % (initMetaData->layer - 1)) + 1;
            }
            segment.layer[brickPtr->layerId].push_back(brickPtr);
        }
    }

    for(uint32_t i = 0; i < segment.layer.size(); i++) {
        std::cout<<"layer "<<i<<": "<<segment.layer[i].size()<<std::endl;
    }

    return true;
}

bool
initializeNodes(CoreSegment &segment,
                Kitsunemimi::Ai::InitMeataData* initMetaData)
{
    const uint32_t numberOfNodes = segment.segmentMeta->numberOfNodeBricks
                                   * segment.segmentMeta->numberOfNodesPerBrick;
    const float range = initMetaData->nodeUpperBorder
                        - initMetaData->nodeLowerBorder;

    for(uint32_t i = 0; i < numberOfNodes; i++)
    {
        segment.nodes[i].border = fmod(static_cast<float>(rand()), range);
        segment.nodes[i].border += initMetaData->nodeLowerBorder;
    }

    return true;
}

/**
 * @brief add new brick
 *
 * @param segment segment where to add a new brick
 */
void addBricksToSegment(CoreSegment &segment,
                        Kitsunemimi::Ai::InitMeataData* initMetaData,
                        const Kitsunemimi::Ai::AiBaseMeta& metaBase)
{
    uint32_t nodeBrickIdCounter = 0;
    uint32_t inputCounter = 0;

    for(uint32_t i = 0; i < metaBase.bricks.size(); i++)
    {
        Brick initBrick;

        // copy metadata
        initBrick.brickId = metaBase.bricks[i].brickId;
        initBrick.nodeBrickId = metaBase.bricks[i].nodeBrickId;
        initBrick.isOutputBrick = metaBase.bricks[i].isOutputBrick;
        initBrick.isInputBrick = metaBase.bricks[i].isInputBrick;

        // copy position
        initBrick.brickPos.x = static_cast<int32_t>(metaBase.bricks[i].brickPos.x);
        initBrick.brickPos.y = static_cast<int32_t>(metaBase.bricks[i].brickPos.y);
        initBrick.brickPos.z = static_cast<int32_t>(metaBase.bricks[i].brickPos.z);

        // copy neighbors
        for(uint32_t j = 0; j < 12; j++) {
            initBrick.neighbors[j] = metaBase.bricks[i].neighbors[j];
        }

        // handle node-brick
        if(initBrick.nodeBrickId != UNINIT_STATE_32)
        {
            const uint32_t nodePos = initBrick.nodeBrickId * initMetaData->nodesPerBrick;
            assert(nodePos < 0x7FFFFFFF);
            initBrick.nodePos = nodePos;

            // handle output-brick
            if(initBrick.isOutputBrick)
            {
                Node* array = segment.nodes;
                for(uint32_t j = 0; j < initMetaData->nodesPerBrick; j++) {
                    array[j + nodePos].border = -2.0f;
                }
            }

            // handle input-brick
            if(initBrick.isInputBrick)
            {
                Node* array = segment.nodes;
                for(uint32_t j = 0; j < initMetaData->nodesPerBrick; j++)
                {
                    array[j + nodePos].border = 0.0f;
                    segment.inputNodes[inputCounter].targetNode = j + nodePos;
                    inputCounter++;
                }
            }

            // copy new brick to segment
            segment.nodeBricks[nodeBrickIdCounter] = initBrick;
            assert(nodeBrickIdCounter == initBrick.nodeBrickId);
            nodeBrickIdCounter++;
        }

        assert(initBrick.brickId == i);
    }

    // add to layer
    if(initMetaData->layer > 0) {
        initLayer(segment, initMetaData);
    }

    return;
}

