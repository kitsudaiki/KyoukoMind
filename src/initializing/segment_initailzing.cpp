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
CoreSegment*
initSynapseSegment(const uint32_t numberOfNodeBricks,
                   const uint32_t numberOfNodes,
                   const uint64_t numberOfSynapseSections,
                   const uint32_t numberOfInputs,
                   const uint32_t numberOfOutputs,
                   const uint32_t numberOfRandValues)
{
    CoreSegment* newSegment = new CoreSegment();

    // get total buffer-size for the new segment
    uint32_t totalSize = 0;
    totalSize += 1 * sizeof(CoreSegmentMeta);
    totalSize += 1 * sizeof(Kitsunemimi::Ai::CoreMetaData);
    totalSize += numberOfNodeBricks * sizeof(Brick);
    totalSize += numberOfNodes * sizeof(Node);
    totalSize += numberOfSynapseSections * sizeof(SynapseSection);
    totalSize += numberOfSynapseSections * sizeof(SynapseBuffer);
    totalSize += numberOfInputs * sizeof(InputNode);
    totalSize += numberOfOutputs * sizeof(OutputNode);

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
    newSegment->segmentMeta->numberOfOutputs = numberOfOutputs;
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

    // init output
    newSegment->outputNodes = reinterpret_cast<OutputNode*>(data + bufferPos);
    bufferPos += numberOfOutputs * sizeof(OutputNode);
    for(uint32_t i = 0; i < numberOfOutputs; i++) {
        newSegment->outputNodes[i] = OutputNode();
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
        Brick* brick = &segment.nodeBricks[i];

        if(brick->isInputBrick)
        {
            brick->layerId = 0;
            segment.layer[brick->layerId].push_back(brick);
        }
        else if(brick->isOutputBrick)
        {
            brick->layerId = initMetaData->layer;
            segment.layer[brick->layerId].push_back(brick);
        }
        else if(brick->nodeBrickId != UNINIT_STATE_32)
        {
            if(initMetaData->layer == 1) {
                brick->layerId = 1;
            } else {
                brick->layerId = (brick->brickPos.x % (initMetaData->layer - 1)) + 1;
            }
            segment.layer[brick->layerId].push_back(brick);
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
            const uint32_t nodePos = newBrick.nodeBrickId * initMetaData->nodesPerBrick;
            assert(nodePos < 0x7FFFFFFF);
            newBrick.nodePos = nodePos;
            newBrick.numberOfNodes = initMetaData->nodesPerBrick;

            // handle output-brick
            if(newBrick.isOutputBrick)
            {
                Node* nodes = segment.nodes;
                for(uint32_t j = 0; j < initMetaData->nodesPerBrick; j++) {
                    nodes[j + nodePos].border = -2.0f;
                }

                for(uint32_t i = 0; i < segment.segmentMeta->numberOfOutputs; i++) {
                    segment.outputNodes[i].targetNode = nodePos + i;
                }

                newBrick.numberOfNodes = segment.segmentMeta->numberOfOutputs;
            }

            // handle input-brick
            if(newBrick.isInputBrick)
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
            segment.nodeBricks[nodeBrickIdCounter] = newBrick;
            assert(nodeBrickIdCounter == newBrick.nodeBrickId);
            nodeBrickIdCounter++;
        }

        assert(newBrick.brickId == i);
    }

    // add to layer
    if(initMetaData->layer > 0) {
        initLayer(segment, initMetaData);
    }

    return;
}

