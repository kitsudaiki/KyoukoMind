/**
 * @file        static_segment.cpp
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

#include "static_segment.h"

#include <core/routing_functions.h>
#include <libKitsunemimiCommon/logger.h>

/**
 * @brief constructor
 */
StaticSegment::StaticSegment()
    : AbstractSegment()
{
    m_type = STATIC_SEGMENT;
}

/**
 * @brief destructor
 */
StaticSegment::~StaticSegment() {}

/**
 * @brief initalize segment
 *
 * @param parsedContent json-object with the segment-description
 *
 * @return true, if successful, else false
 */
bool
StaticSegment::initSegment(const JsonItem &parsedContent)
{
    // parse bricks
    const JsonItem paredLayer = parsedContent.get("bricks");
    const uint32_t numberOfNodeBricks = paredLayer.size();

    // get nodes
    uint32_t numberOfNodes = 0;
    for(uint32_t i = 0; i < numberOfNodeBricks; i++) {
        numberOfNodes += paredLayer.get(i).get("number_of_nodes").getInt();
    }

    // get connections
    uint32_t numberOfConnections = 0;
    for(uint32_t i = 1; i < numberOfNodeBricks; i++)
    {
        const uint32_t lastLayer = paredLayer.get(i - 1).get("number_of_nodes").getInt();
        const uint32_t actualLayer = paredLayer.get(i).get("number_of_nodes").getInt();
        numberOfConnections += lastLayer * actualLayer;
    }
    const uint32_t totalBorderSize = parsedContent.get("total_border_size").getInt();

    // create segment metadata
    const StaticSegmentSettings settings = initSettings(parsedContent);
    SegmentHeader header = createNewHeader(numberOfNodeBricks,
                                           numberOfConnections,
                                           numberOfNodes,
                                           totalBorderSize);
    // initialize segment itself
    allocateSegment(header);
    initSegmentPointer(header);
    initDefaultValues();
    segmentHeader->position = convertPosition(parsedContent);
    staticSegmentSettings[0] = settings;

    // init content
    addBricksToSegment(parsedContent);
    initializeNodes();

    // init border
    initBorderBuffer(parsedContent);
    connectBorderBuffer();

    return true;
}

/**
 * @brief init border-buffer
 *
 * @return true, if successful, else false
 */
bool
StaticSegment::connectBorderBuffer()
{
    StaticNode* node = nullptr;
    Brick* brick = nullptr;

    uint64_t transferCounter = 0;

    for(uint32_t i = 0; i < segmentHeader->bricks.count; i++)
    {
        brick = &bricks[i];

        // connect input-bricks with border-buffer
        if(brick->isInputBrick)
        {
            for(uint32_t j = 0; j < brick->numberOfNodes; j++)
            {
                if(transferCounter >= segmentHeader->inputTransfers.count) {
                    break;
                }
                node = &nodes[brick->nodePos + j];
                node->targetBorderId = transferCounter;
                transferCounter++;
            }
        }

        // connect output-bricks with border-buffer
        if(brick->isOutputBrick)
        {
            for(uint32_t j = 0; j < brick->numberOfNodes; j++)
            {
                if(transferCounter >= segmentHeader->outputTransfers.count) {
                    break;
                }
                node = &nodes[brick->nodePos + j];
                node->targetBorderId = transferCounter;
                transferCounter++;
            }
        }
    }

    return true;
}

/**
 * @brief init sttings-block for the segment
 *
 * @param parsedContent json-object with the segment-description
 *
 * @return settings-object
 */
StaticSegmentSettings
StaticSegment::initSettings(const JsonItem &)
{
    StaticSegmentSettings settings;

    return settings;
}

/**
 * @brief create new segment-header with size and position information
 *
 * @param numberOfBricks number of bricks
 * @param numberOfConnections number of connections
 * @param numberOfNodes number of nodes
 * @param borderbufferSize size of border-buffer
 *
 * @return new segment-header
 */
SegmentHeader
StaticSegment::createNewHeader(const uint32_t numberOfBricks,
                               const uint32_t numberOfConnections,
                               const uint32_t numberOfNodes,
                               const uint64_t borderbufferSize)
{
    SegmentHeader segmentHeader;
    segmentHeader.segmentType = m_type;
    uint32_t segmentDataPos = createGenericNewHeader(segmentHeader, borderbufferSize);

    // init bricks
    segmentHeader.bricks.count = numberOfBricks;
    segmentHeader.bricks.bytePos = segmentDataPos;
    segmentDataPos += numberOfBricks * sizeof(Brick);

    // init connections
    segmentHeader.connections.count = numberOfConnections;
    segmentHeader.connections.bytePos = segmentDataPos;
    segmentDataPos += numberOfConnections * sizeof(float);

    // init nodes
    segmentHeader.nodes.count = numberOfNodes;
    segmentHeader.nodes.bytePos = segmentDataPos;
    segmentDataPos += numberOfNodes * sizeof(StaticNode);

    segmentHeader.staticDataSize = segmentDataPos;

    return segmentHeader;
}

/**
 * @brief init pointer within the segment-header
 *
 * @param header segment-header
 */
void
StaticSegment::initSegmentPointer(const SegmentHeader &header)
{
    uint8_t* dataPtr = static_cast<uint8_t*>(staticSegmentData.data);
    uint64_t pos = 0;

    segmentHeader = reinterpret_cast<SegmentHeader*>(dataPtr + pos);
    segmentHeader[0] = header;

    pos = 256;
    staticSegmentSettings = reinterpret_cast<StaticSegmentSettings*>(dataPtr + pos);
    pos = segmentHeader->neighborList.bytePos;
    segmentNeighbors = reinterpret_cast<SegmentNeighborList*>(dataPtr + pos);
    pos = segmentHeader->inputTransfers.bytePos;
    inputTransfers = reinterpret_cast<float*>(dataPtr + pos);
    pos = segmentHeader->outputTransfers.bytePos;
    outputTransfers = reinterpret_cast<float*>(dataPtr + pos);
    pos = segmentHeader->bricks.bytePos;
    bricks = reinterpret_cast<Brick*>(dataPtr + pos);
    pos = segmentHeader->nodes.bytePos;
    nodes = reinterpret_cast<StaticNode*>(dataPtr + pos);
    pos = segmentHeader->connections.bytePos;
    connections = reinterpret_cast<float*>(dataPtr + pos);
}

/**
 * @brief allocate memory for the segment
 *
 * @param header header with the size-information
 */
void
StaticSegment::allocateSegment(SegmentHeader &header)
{
    const uint32_t numberOfBlocks = (header.staticDataSize / 4096) + 1;
    header.staticDataSize = numberOfBlocks * 4096;
    Kitsunemimi::allocateBlocks_DataBuffer(staticSegmentData, numberOfBlocks);
}

/**
 * @brief init buffer to avoid undefined values
 */
void
StaticSegment::initDefaultValues()
{
    // init bricks;
    for(uint32_t i = 0; i < segmentHeader->bricks.count; i++) {
        bricks[i] = Brick();
    }

    // init nodes;
    for(uint32_t i = 0; i < segmentHeader->nodes.count; i++) {
        nodes[i] = StaticNode();
    }

    // init connections
    for(uint32_t i = 0; i < segmentHeader->connections.count; i++)
    {
        connections[i] = static_cast<float>(rand() % 10000);
        connections[i] /= 10000.0f;
        connections[i] -= 0.5f;
    }
}

/**
 * @brief create a new brick-object
 *
 * @param brickDef json with all brick-definitions
 * @param id brick-id
 *
 * @return new brick with parsed information
 */
Brick
StaticSegment::createNewBrick(const JsonItem &brickDef, const uint32_t id)
{
    Brick newBrick;

    // copy metadata
    newBrick.brickId = id;
    if(brickDef.contains("type"))
    {
        if(brickDef.get("type").getString() == "output") {
            newBrick.isOutputBrick = true;
        }
        if(brickDef.get("type").getString() == "input") {
            newBrick.isInputBrick = true;
        }
    }

    // convert other values
    newBrick.brickPos = convertPosition(brickDef);
    newBrick.numberOfNodes = brickDef.get("number_of_nodes").getInt();
    for(uint8_t side = 0; side < 12; side++) {
        newBrick.neighbors[side] = UNINIT_STATE_32;
    }

    return newBrick;
}

/**
 * @brief init all bricks
 *
 * @param metaBase json with all brick-definitions
 */
void
StaticSegment::addBricksToSegment(const JsonItem &metaBase)
{
    uint32_t nodeBrickIdCounter = 0;
    uint32_t nodePosCounter = 0;
    const JsonItem brickDef = metaBase.get("bricks");

    for(uint32_t i = 0; i < brickDef.size(); i++)
    {
        Brick newBrick = createNewBrick(brickDef.get(i), i);

        // handle node-brick
        newBrick.nodePos = nodePosCounter;
        for(uint32_t j = 0; j < newBrick.numberOfNodes; j++) {
            nodes[j + nodePosCounter].brickId = newBrick.brickId;
        }

        // copy new brick to segment
        bricks[nodeBrickIdCounter] = newBrick;
        assert(nodeBrickIdCounter == newBrick.brickId);
        nodeBrickIdCounter++;
        nodePosCounter += newBrick.numberOfNodes;
    }

    return;
}

/**
 * @brief init all nodes with activation-border
 *
 * @return true, if successful, else false
 */
bool
StaticSegment::initializeNodes()
{
    const float range = 1.0f;
    uint32_t nodePos = bricks[0].numberOfNodes;
    uint32_t connectionPos = 0;

    for(uint32_t i = 1; i < segmentHeader->bricks.count; i++)
    {
        const uint32_t numberOfNodes = bricks[i].numberOfNodes;
        const uint32_t numberOfNodesLast = bricks[i - 1].numberOfNodes;

        for(uint32_t nodeId = nodePos;
            nodeId < numberOfNodes + nodePos;
            nodeId++)
        {
            nodes[nodeId].numberOfConnections = numberOfNodesLast;
            nodes[nodeId].targetConnectionPos = connectionPos;
            //nodes[nodeId].border = fmod(static_cast<float>(rand()), range);
            //nodes[nodeId].border -= 0.5f;
            nodes[nodeId].border = 0.0f;

            connectionPos += numberOfNodesLast;
        }

        nodePos += numberOfNodes;
    }

    return true;
}
