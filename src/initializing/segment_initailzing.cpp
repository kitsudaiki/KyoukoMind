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
    uint32_t persistentBufferPos = 0;

    // init header
    persistentBufferPos += 1 * sizeof(SegmentHeader);

    // init settings
    segmentHeader.settings.count = 1;
    segmentHeader.settings.bytePos = persistentBufferPos;
    persistentBufferPos += 1 * sizeof(Kitsunemimi::Ai::SegmentSettings);

    // init bricks
    segmentHeader.bricks.count = numberOfBricks;
    segmentHeader.bricks.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfBricks * sizeof(Brick);

    // init brick-order
    segmentHeader.brickOrder.count = numberOfBricks;
    segmentHeader.brickOrder.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfBricks * sizeof(uint32_t);

    // init nodes
    segmentHeader.nodes.count = numberOfNodes;
    segmentHeader.nodes.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfNodes * sizeof(Node);

    // init synapse sections
    segmentHeader.synapseSections.count = numberOfSynapseSections;
    segmentHeader.synapseSections.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfSynapseSections * sizeof(SynapseSection);


    uint32_t ephemeralBufferPos = 0;

    // init input
    segmentHeader.inputs.count = numberOfInputs;
    segmentHeader.inputs.bytePos = ephemeralBufferPos;
    ephemeralBufferPos += numberOfInputs * sizeof(InputNode);

    // init output
    segmentHeader.outputs.count = numberOfOutputs;
    segmentHeader.outputs.bytePos = ephemeralBufferPos;
    ephemeralBufferPos += numberOfOutputs * sizeof(OutputNode);


    segmentHeader.segmentSize = persistentBufferPos;
    segmentHeader.segmentPersistentBufferSize = persistentBufferPos;
    segmentHeader.segmentEphemeralBufferSize = ephemeralBufferPos;

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
    uint8_t* persistentData = static_cast<uint8_t*>(segment.persistenBuffer.data);

    segment.segmentHeader = reinterpret_cast<SegmentHeader*>(persistentData + 0);
    segment.segmentHeader[0] = header;

    segment.synapseSettings = reinterpret_cast<Kitsunemimi::Ai::SegmentSettings*>(persistentData + 256);
    segment.bricks = reinterpret_cast<Brick*>(persistentData + segment.segmentHeader->bricks.bytePos);
    segment.brickOrder = reinterpret_cast<uint32_t*>(persistentData + segment.segmentHeader->brickOrder.bytePos);
    segment.nodes = reinterpret_cast<Node*>(persistentData + segment.segmentHeader->nodes.bytePos);
    segment.synapseSections = reinterpret_cast<SynapseSection*>(persistentData + segment.segmentHeader->synapseSections.bytePos);

    uint8_t* ephemeralData = static_cast<uint8_t*>(segment.ephemeralBuffer.data);

    segment.inputs = reinterpret_cast<InputNode*>(ephemeralData + segment.segmentHeader->inputs.bytePos);
    segment.outputs = reinterpret_cast<OutputNode*>(ephemeralData + segment.segmentHeader->outputs.bytePos);
}

/**
 * @brief allocateSegment
 * @param header
 * @return
 */
Segment*
allocateSegment(SegmentHeader &header)
{
    Segment* newSegment = new Segment();

    // persistent part
    const uint32_t numberOfPersistentBlocks = (header.segmentPersistentBufferSize / 4096) + 1;
    header.segmentPersistentBufferSize = numberOfPersistentBlocks * 4096;
    Kitsunemimi::allocateBlocks_DataBuffer(newSegment->persistenBuffer, numberOfPersistentBlocks);

    // ephemeral part
    const uint32_t numberOfEphemeralBlocks = (header.segmentEphemeralBufferSize / 4096) + 1;
    header.segmentEphemeralBufferSize = numberOfEphemeralBlocks * 4096;
    Kitsunemimi::allocateBlocks_DataBuffer(newSegment->ephemeralBuffer, numberOfEphemeralBlocks);

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
    segment->synapseSettings[0] = Kitsunemimi::Ai::SegmentSettings();

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

    // init synapse sections
    for(uint32_t i = 0; i < numberOfSynapseSections; i++)
    {
        segment->synapseSections[i] = SynapseSection();
        segment->synapseSections[i].randomPos = static_cast<uint32_t>(rand()) % numberOfRandValues;
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

/**
 * @brief initializeNodes
 * @param segment
 * @param initMetaData
 * @return
 */
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
 * @brief addBricksToSegment
 * @param segment
 * @param initMetaData
 * @param metaBase
 */
void
addBricksToSegment(Segment &segment,
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

