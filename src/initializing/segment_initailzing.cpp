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
 * @brief createNewHeader
 * @param numberOfBricks
 * @param numberOfNodes
 * @param numberOfSynapseSections
 * @param numberOfInputs
 * @param numberOfOutputs
 * @return
 */
SegmentHeader
createNewHeader(const uint32_t numberOfBricks,
                const uint32_t numberOfNodes,
                const uint64_t numberOfSynapseSections,
                const uint32_t numberOfInputs,
                const uint32_t numberOfOutputs)
{
    SegmentHeader segmentHeader;
    uint32_t bufferPos = 0;

    // init header
    bufferPos += 1 * sizeof(SegmentHeader);

    // init settings
    segmentHeader.settings.count = 1;
    segmentHeader.settings.bytePos = bufferPos;
    bufferPos += 1 * sizeof(Kitsunemimi::Ai::SegmentSettings);

    // init bricks
    segmentHeader.bricks.count = numberOfBricks;
    segmentHeader.bricks.bytePos = bufferPos;
    bufferPos += numberOfBricks * sizeof(Brick);

    // init brick-order
    segmentHeader.brickOrder.count = numberOfBricks;
    segmentHeader.brickOrder.bytePos = bufferPos;
    bufferPos += numberOfBricks * sizeof(uint32_t);

    // init nodes
    segmentHeader.nodes.count = numberOfNodes;
    segmentHeader.nodes.bytePos = bufferPos;
    bufferPos += numberOfNodes * sizeof(Node);

    // init nodes-buffers
    segmentHeader.nodeBuffers.count = numberOfNodes / numberOfBricks;
    segmentHeader.nodeBuffers.bytePos = bufferPos;
    bufferPos += numberOfBricks * 127 * sizeof(float);

    // init synapse sections
    segmentHeader.synapseSections.count = numberOfSynapseSections;
    segmentHeader.synapseSections.bytePos = bufferPos;
    bufferPos += numberOfSynapseSections * sizeof(SynapseSection);

    // init synapse buffer
    segmentHeader.synapseBuffers.count = numberOfSynapseSections;
    segmentHeader.synapseBuffers.bytePos = bufferPos;
    bufferPos += numberOfSynapseSections * sizeof(SynapseBuffer);

    // init input
    segmentHeader.inputs.count = numberOfInputs;
    segmentHeader.inputs.bytePos = bufferPos;
    bufferPos += numberOfInputs * sizeof(InputNode);

    // init output
    segmentHeader.outputs.count = numberOfOutputs;
    segmentHeader.outputs.bytePos = bufferPos;
    bufferPos += numberOfOutputs * sizeof(OutputNode);

    segmentHeader.segmentSize = bufferPos;

    return segmentHeader;
}

/**
 * @brief initSegmentPointer
 * @param segment
 * @param header
 */
void
initSegmentPointer(Segment &segment,
                   const SegmentHeader &header)
{
    uint8_t* data = static_cast<uint8_t*>(segment.buffer.data);

    segment.segmentHeader = reinterpret_cast<SegmentHeader*>(data + 0);
    segment.segmentHeader[0] = header;

    segment.synapseMetaData = reinterpret_cast<Kitsunemimi::Ai::SegmentSettings*>(data + 256);
    segment.bricks = reinterpret_cast<Brick*>(data + segment.segmentHeader->bricks.bytePos);
    segment.brickOrder = reinterpret_cast<uint32_t*>(data + segment.segmentHeader->brickOrder.bytePos);
    segment.nodes = reinterpret_cast<Node*>(data + segment.segmentHeader->nodes.bytePos);
    segment.nodeBuffers = reinterpret_cast<float*>(data + segment.segmentHeader->nodeBuffers.bytePos);
    segment.synapseSections = reinterpret_cast<SynapseSection*>(data + segment.segmentHeader->synapseSections.bytePos);
    segment.synapseBuffers = reinterpret_cast<SynapseBuffer*>(data + segment.segmentHeader->synapseBuffers.bytePos);
    segment.inputs = reinterpret_cast<InputNode*>(data + segment.segmentHeader->inputs.bytePos);
    segment.outputs = reinterpret_cast<OutputNode*>(data + segment.segmentHeader->outputs.bytePos);
}

/**
 * @brief allocateSegment
 * @param header
 * @return
 */
Segment*
allocateSegment(const SegmentHeader &header)
{
    Segment* newSegment = new Segment();

    const uint32_t numberOfBlocks = (header.segmentSize / 4096) + 1;
    Kitsunemimi::allocateBlocks_DataBuffer(newSegment->buffer, numberOfBlocks);
    initSegmentPointer(*newSegment, header);

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
Segment*
initSynapseSegment(const uint32_t numberOfBricks,
                   const uint32_t numberOfNodes,
                   const uint64_t numberOfSynapseSections,
                   const uint32_t numberOfInputs,
                   const uint32_t numberOfOutputs,
                   const uint32_t numberOfRandValues)
{
    SegmentHeader header = createNewHeader(numberOfBricks,
                                           numberOfNodes,
                                           numberOfSynapseSections,
                                           numberOfInputs,
                                           numberOfOutputs);
    Segment* segment = allocateSegment(header);

    // init header and metadata
    segment->synapseMetaData[0] = Kitsunemimi::Ai::SegmentSettings();

    // init bricks;
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        segment->bricks[i] = Brick();
    }

    // init brick-order
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        segment->brickOrder[i] = i;
    }

    // init nodes
    for(uint32_t i = 0; i < numberOfNodes; i++) {
        segment->nodes[i] = Node();
    }

    // init nodes-buffers
    for(uint32_t i = 0; i < numberOfBricks * 127; i++) {
        segment->nodeBuffers[i] = 0.0f;
    }

    // init synapse sections
    for(uint32_t i = 0; i < numberOfSynapseSections; i++)
    {
        segment->synapseSections[i] = SynapseSection();
        segment->synapseSections[i].randomPos = static_cast<uint32_t>(rand()) % numberOfRandValues;
    }

    // init synapse buffer
    for(uint32_t i = 0; i < numberOfSynapseSections; i++) {
        segment->synapseBuffers[i] = SynapseBuffer();
    }

    // init input
    for(uint32_t i = 0; i < numberOfInputs; i++) {
        segment->inputs[i] = InputNode();
    }

    // init output
    for(uint32_t i = 0; i < numberOfOutputs; i++) {
        segment->outputs[i] = OutputNode();
    }

    return segment;
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

