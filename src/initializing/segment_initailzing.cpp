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
 * @brief Segment::initSynapseSegment
 * @param numberOfNodeBricks
 * @param numberOfNodes
 * @param numberOfSynapseSections
 * @param numberOfRandValues
 * @return
 */
Segment*
initSynapseSegment(const uint32_t numberOfBricks,
                   const uint32_t numberOfNodes,
                   const uint64_t numberOfSynapseSections,
                   const uint32_t numberOfInputs,
                   const uint32_t numberOfOutputs,
                   const uint32_t numberOfRandValues)
{
    Segment* newSegment = new Segment();

    // get total buffer-size for the new segment
    uint32_t totalSize = 0;
    totalSize += 1 * sizeof(SegmentHeader);
    totalSize += 1 * sizeof(Kitsunemimi::Ai::SegmentSettings);
    totalSize += numberOfBricks * sizeof(Brick);
    totalSize += numberOfBricks * sizeof(uint32_t);
    totalSize += numberOfNodes * sizeof(Node);
    totalSize += (numberOfNodes / numberOfBricks) * 127 * sizeof(float);
    totalSize += numberOfSynapseSections * sizeof(SynapseSection);
    totalSize += numberOfSynapseSections * sizeof(SynapseBuffer);
    totalSize += numberOfInputs * sizeof(InputNode);
    totalSize += numberOfOutputs * sizeof(OutputNode);

    // allocate memory
    const uint32_t numberOfBlocks = (totalSize / 4096) + 1;
    Kitsunemimi::allocateBlocks_DataBuffer(newSegment->buffer, numberOfBlocks);

    uint8_t* data = static_cast<uint8_t*>(newSegment->buffer.data);
    uint32_t bufferPos = 0;

    // init header
    newSegment->segmentHeader = reinterpret_cast<SegmentHeader*>(data + bufferPos);
    bufferPos += 1 * sizeof(SegmentHeader);
    newSegment->segmentHeader[0] = SegmentHeader();

    // init settings
    newSegment->segmentHeader->settings.count = 1;
    newSegment->segmentHeader->settings.bytePos = bufferPos;
    newSegment->synapseMetaData = reinterpret_cast<Kitsunemimi::Ai::SegmentSettings*>(data + bufferPos);
    bufferPos += 1 * sizeof(Kitsunemimi::Ai::SegmentSettings);
    newSegment->synapseMetaData[0] = Kitsunemimi::Ai::SegmentSettings();

    // init bricks
    newSegment->segmentHeader->bricks.count = numberOfBricks;
    newSegment->segmentHeader->bricks.bytePos = bufferPos;
    newSegment->bricks = reinterpret_cast<Brick*>(data + bufferPos);
    bufferPos += numberOfBricks * sizeof(Brick);
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        newSegment->bricks[i] = Brick();
    }

    // init brick-order
    newSegment->segmentHeader->brickOrder.count = numberOfBricks;
    newSegment->segmentHeader->brickOrder.bytePos = bufferPos;
    newSegment->brickOrder = reinterpret_cast<uint32_t*>(data + bufferPos);
    bufferPos += numberOfBricks * sizeof(uint32_t);
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        newSegment->brickOrder[i] = i;
    }

    // init nodes
    newSegment->segmentHeader->nodes.count = numberOfNodes;
    newSegment->segmentHeader->nodes.bytePos = bufferPos;
    newSegment->nodes = reinterpret_cast<Node*>(data + bufferPos);
    bufferPos += numberOfNodes * sizeof(Node);
    for(uint32_t i = 0; i < numberOfNodes; i++) {
        newSegment->nodes[i] = Node();
    }

    // init nodes-buffers
    newSegment->segmentHeader->nodeBuffers.count = numberOfNodes / numberOfBricks;
    newSegment->segmentHeader->nodeBuffers.bytePos = bufferPos;
    newSegment->nodeBuffers = reinterpret_cast<float*>(data + bufferPos);
    bufferPos += numberOfBricks * 127 * sizeof(float);
    for(uint32_t i = 0; i < numberOfBricks * 127; i++) {
        newSegment->nodeBuffers[i] = 0.0f;
    }

    // init synapse sections
    newSegment->segmentHeader->synapseSections.count = numberOfSynapseSections;
    newSegment->segmentHeader->synapseSections.bytePos = bufferPos;
    newSegment->synapseSections = reinterpret_cast<SynapseSection*>(data + bufferPos);
    bufferPos += numberOfSynapseSections * sizeof(SynapseSection);
    for(uint32_t i = 0; i < numberOfSynapseSections; i++)
    {
        newSegment->synapseSections[i] = SynapseSection();
        newSegment->synapseSections[i].randomPos = static_cast<uint32_t>(rand()) % numberOfRandValues;
    }

    // init synapse buffer
    newSegment->segmentHeader->synapseBuffers.count = numberOfSynapseSections;
    newSegment->segmentHeader->synapseBuffers.bytePos = bufferPos;
    newSegment->synapseBuffers = reinterpret_cast<SynapseBuffer*>(data + bufferPos);
    bufferPos += numberOfSynapseSections * sizeof(SynapseBuffer);
    for(uint32_t i = 0; i < numberOfSynapseSections; i++) {
        newSegment->synapseBuffers[i] = SynapseBuffer();
    }

    // init input
    newSegment->segmentHeader->inputs.count = numberOfInputs;
    newSegment->segmentHeader->inputs.bytePos = bufferPos;
    newSegment->inputs = reinterpret_cast<InputNode*>(data + bufferPos);
    bufferPos += numberOfInputs * sizeof(InputNode);
    for(uint32_t i = 0; i < numberOfInputs; i++) {
        newSegment->inputs[i] = InputNode();
    }

    // init output
    newSegment->segmentHeader->outputs.count = numberOfOutputs;
    newSegment->segmentHeader->outputs.bytePos = bufferPos;
    newSegment->outputs = reinterpret_cast<OutputNode*>(data + bufferPos);
    bufferPos += numberOfOutputs * sizeof(OutputNode);
    for(uint32_t i = 0; i < numberOfOutputs; i++) {
        newSegment->outputs[i] = OutputNode();
    }

    return newSegment;
}

bool
initializeNodes(Segment &segment,
                Kitsunemimi::Ai::InitSettings* initMetaData)
{
    const uint32_t numberOfNodes = segment.segmentHeader->nodes.count;
    const float range = initMetaData->nodeUpperBorder - initMetaData->nodeLowerBorder;

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
void addBricksToSegment(Segment &segment,
                        Kitsunemimi::Ai::InitSettings* initMetaData,
                        const Kitsunemimi::Ai::AiBaseMeta& metaBase)
{
    uint32_t nodeBrickIdCounter = 0;
    uint32_t inputCounter = 0;

    for(uint32_t i = 0; i < metaBase.bricks.size(); i++)
    {
        Brick newBrick;

        // copy metadata
        newBrick.brickId = metaBase.bricks[i].brickId;
        newBrick.nodeBrickId = metaBase.bricks[i].nodeBrickId;
        newBrick.isOutputBrick = metaBase.bricks[i].isOutputBrick;
        newBrick.isInputBrick = metaBase.bricks[i].isInputBrick;

        // copy position
        newBrick.brickPos.x = static_cast<int32_t>(metaBase.bricks[i].brickPos.x);
        newBrick.brickPos.y = static_cast<int32_t>(metaBase.bricks[i].brickPos.y);
        newBrick.brickPos.z = static_cast<int32_t>(metaBase.bricks[i].brickPos.z);

        // copy neighbors
        for(uint32_t j = 0; j < 12; j++) {
            newBrick.neighbors[j] = metaBase.bricks[i].neighbors[j];
        }

        // handle node-brick
        if(newBrick.nodeBrickId != UNINIT_STATE_32)
        {
            const uint32_t nodeOffset = newBrick.nodeBrickId * initMetaData->nodesPerBrick;
            assert(nodeOffset < 0x7FFFFFFF);
            newBrick.nodePos = nodeOffset;
            newBrick.numberOfNodes = initMetaData->nodesPerBrick;

            // handle output-brick
            if(newBrick.isOutputBrick)
            {
                Node* nodes = segment.nodes;
                for(uint32_t j = 0; j < initMetaData->nodesPerBrick; j++) {
                    nodes[j + nodeOffset].border = -2.0f;
                }

                for(uint32_t i = 0; i < segment.segmentHeader->outputs.count; i++) {
                    segment.outputs[i].targetNode = nodeOffset + i;
                }

                newBrick.numberOfNodes = segment.segmentHeader->outputs.count;
            }

            // handle input-brick
            if(newBrick.isInputBrick)
            {
                Node* array = segment.nodes;
                for(uint32_t j = 0; j < initMetaData->nodesPerBrick; j++)
                {
                    array[j + nodeOffset].border = 0.0f;
                    segment.inputs[inputCounter].targetNode = j + nodeOffset;
                    inputCounter++;
                }
            }

            Node* nodes = segment.nodes;
            for(uint32_t j = 0; j < initMetaData->nodesPerBrick; j++) {
                nodes[j + nodeOffset].nodeBrickId = newBrick.nodeBrickId;
            }

            // copy new brick to segment
            segment.bricks[nodeBrickIdCounter] = newBrick;
            assert(nodeBrickIdCounter == newBrick.nodeBrickId);
            nodeBrickIdCounter++;
        }

        assert(newBrick.brickId == i);
    }

    return;
}

