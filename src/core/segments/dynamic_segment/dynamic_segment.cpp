/**
 * @file        dynamic_segment.cpp
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

#include "dynamic_segment.h"

#include <core/routing_functions.h>
#include <libKitsunemimiCommon/logger.h>

/**
 * @brief constructor
 */
DynamicSegment::DynamicSegment()
    : AbstractSegment()
{
    m_type = DYNAMIC_SEGMENT;
}

DynamicSegment::DynamicSegment(const void* data, const uint64_t dataSize)
    : AbstractSegment(data, dataSize)
{
    m_type = DYNAMIC_SEGMENT;
}

/**
 * @brief destructor
 */
DynamicSegment::~DynamicSegment() {}

/**
 * @brief initalize segment
 *
 * @param parsedContent json-object with the segment-description
 *
 * @return true, if successful, else false
 */
bool
DynamicSegment::initSegment(const JsonItem &parsedContent)
{
    const std::string name = parsedContent.get("name").getString();
    const JsonItem paredBricks = parsedContent.get("bricks");
    const uint32_t numberOfNodeBricks = paredBricks.size();
    uint32_t totalNumberOfNodes = 0;
    for(uint32_t i = 0; i < numberOfNodeBricks; i++) {
        totalNumberOfNodes += paredBricks.get(i).get("number_of_nodes").getInt();
    }
    const uint32_t totalBorderSize = parsedContent.get("total_border_size").getInt();

    // create segment metadata
    const DynamicSegmentSettings settings = initSettings(parsedContent);
    SegmentHeader header = createNewHeader(numberOfNodeBricks,
                                           totalNumberOfNodes,
                                           settings.maxSynapseSections,
                                           totalBorderSize);
    header.position = convertPosition(parsedContent);

    // initialize segment itself
    allocateSegment(header);
    initSegmentPointer(header);
    initDefaultValues();
    dynamicSegmentSettings[0] = settings;

    // init content
    initializeNodes();
    addBricksToSegment(parsedContent);
    connectAllBricks();
    initTargetBrickList();

    // init border
    initBorderBuffer(parsedContent);
    connectBorderBuffer();

    // TODO: check result
    setName(name);

    return true;
}

/**
 * @brief DynamicSegment::reinitPointer
 * @return
 */
bool
DynamicSegment::reinitPointer(const uint64_t numberOfBytes)
{
    // TODO: checks
    uint8_t* dataPtr = static_cast<uint8_t*>(segmentData.staticData);

    uint64_t pos = 0;
    uint64_t byteCounter = 0;
    segmentHeader = reinterpret_cast<SegmentHeader*>(dataPtr + pos);
    byteCounter += sizeof(SegmentHeader);

    pos = segmentHeader->name.bytePos;
    segmentName = reinterpret_cast<SegmentName*>(dataPtr + pos);
    byteCounter += sizeof(SegmentName);

    pos = segmentHeader->settings.bytePos;
    dynamicSegmentSettings = reinterpret_cast<DynamicSegmentSettings*>(dataPtr + pos);
    byteCounter += sizeof(DynamicSegmentSettings);

    pos = segmentHeader->neighborList.bytePos;
    segmentNeighbors = reinterpret_cast<SegmentNeighborList*>(dataPtr + pos);
    byteCounter += segmentHeader->neighborList.count * sizeof(SegmentNeighborList);

    pos = segmentHeader->inputTransfers.bytePos;
    inputTransfers = reinterpret_cast<float*>(dataPtr + pos);
    byteCounter += segmentHeader->inputTransfers.count * sizeof(float);

    pos = segmentHeader->outputTransfers.bytePos;
    outputTransfers = reinterpret_cast<float*>(dataPtr + pos);
    byteCounter += segmentHeader->outputTransfers.count * sizeof(float);

    pos = segmentHeader->bricks.bytePos;
    bricks = reinterpret_cast<Brick*>(dataPtr + pos);
    byteCounter += segmentHeader->bricks.count * sizeof(Brick);

    pos = segmentHeader->brickOrder.bytePos;
    brickOrder = reinterpret_cast<uint32_t*>(dataPtr + pos);
    byteCounter += segmentHeader->brickOrder.count * sizeof(uint32_t);

    pos = segmentHeader->nodes.bytePos;
    nodes = reinterpret_cast<DynamicNode*>(dataPtr + pos);
    byteCounter += segmentHeader->nodes.count * sizeof(DynamicNode);

    dataPtr = static_cast<uint8_t*>(segmentData.itemData);
    //pos = segmentHeader->synapseSections.bytePos;
    synapseSections = reinterpret_cast<SynapseSection*>(dataPtr);
    byteCounter += segmentHeader->synapseSections.count * sizeof(SynapseSection);

    // check result
    if(byteCounter != numberOfBytes) {
        return false;
    }

    return true;
}

/**
 * @brief init all nodes with activation-border
 *
 * @return true, if successful, else false
 */
bool
DynamicSegment::initializeNodes()
{
    const uint32_t numberOfNodes = segmentHeader->nodes.count;

    for(uint32_t i = 0; i < numberOfNodes; i++)
    {
        //nodes[i].border = ((static_cast<float>(rand() % 10000)) / 10000.0f) * range;
        //nodes[i].border -= 0.25f;
        nodes[i].border = 0.0f;
    }

    return true;
}

/**
 * @brief init border-buffer
 *
 * @return true, if successful, else false
 */
bool
DynamicSegment::connectBorderBuffer()
{
    DynamicNode* node = nullptr;
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
        if(brick->isOutputBrick
                || brick->isTransactionBrick)
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
DynamicSegmentSettings
DynamicSegment::initSettings(const JsonItem &parsedContent)
{
    DynamicSegmentSettings settings;

    // parse settings
    JsonItem paredSettings = parsedContent.get("settings");
    settings.synapseDeleteBorder = paredSettings.get("synapse_delete_border").getFloat();
    settings.nodeCooldown = paredSettings.get("node_cooldown").getFloat();
    settings.memorizing = paredSettings.get("memorizing").getFloat();
    settings.gliaValue = paredSettings.get("glia_value").getFloat();
    settings.maxSynapseWeight = paredSettings.get("max_synapse_weight").getFloat();
    settings.refractionTime = paredSettings.get("refraction_time").getInt();
    settings.signNeg = paredSettings.get("sign_neg").getFloat();
    settings.potentialOverflow = paredSettings.get("potential_overflow").getFloat();
    settings.maxSynapseSections = paredSettings.get("max_synapse_sections").getInt();

    return settings;
}

/**
 * @brief create new segment-header with size and position information
 *
 * @param numberOfBricks number of bricks
 * @param numberOfNodes number of nodes
 * @param numberOfSynapseSections number of synapse-sections
 * @param borderbufferSize size of border-buffer
 *
 * @return new segment-header
 */
SegmentHeader
DynamicSegment::createNewHeader(const uint32_t numberOfBricks,
                                const uint32_t numberOfNodes,
                                const uint64_t numberOfSynapseSections,
                                const uint64_t borderbufferSize)
{
    SegmentHeader segmentHeader;
    segmentHeader.segmentType = m_type;
    uint32_t segmentDataPos = createGenericNewHeader(segmentHeader, borderbufferSize);

    // init bricks
    segmentHeader.bricks.count = numberOfBricks;
    segmentHeader.bricks.bytePos = segmentDataPos;
    segmentDataPos += numberOfBricks * sizeof(Brick);

    // init brick-order
    segmentHeader.brickOrder.count = numberOfBricks;
    segmentHeader.brickOrder.bytePos = segmentDataPos;
    segmentDataPos += numberOfBricks * sizeof(uint32_t);

    // init nodes
    segmentHeader.nodes.count = numberOfNodes;
    segmentHeader.nodes.bytePos = segmentDataPos;
    segmentDataPos += numberOfNodes * sizeof(DynamicNode);

    segmentHeader.staticDataSize = segmentDataPos;

    // init synapse sections
    segmentDataPos = 0;
    segmentHeader.synapseSections.count = numberOfSynapseSections;
    segmentHeader.synapseSections.bytePos = segmentDataPos;

    return segmentHeader;
}

/**
 * @brief init pointer within the segment-header
 *
 * @param header segment-header
 */
void
DynamicSegment::initSegmentPointer(const SegmentHeader &header)
{
    uint8_t* dataPtr = static_cast<uint8_t*>(segmentData.staticData);
    uint64_t pos = 0;

    segmentHeader = reinterpret_cast<SegmentHeader*>(dataPtr + pos);
    segmentHeader[0] = header;

    pos = segmentHeader->name.bytePos;
    segmentName = reinterpret_cast<SegmentName*>(dataPtr + pos);

    pos = segmentHeader->settings.bytePos;
    dynamicSegmentSettings = reinterpret_cast<DynamicSegmentSettings*>(dataPtr + pos);

    pos = segmentHeader->neighborList.bytePos;
    segmentNeighbors = reinterpret_cast<SegmentNeighborList*>(dataPtr + pos);

    pos = segmentHeader->inputTransfers.bytePos;
    inputTransfers = reinterpret_cast<float*>(dataPtr + pos);

    pos = segmentHeader->outputTransfers.bytePos;
    outputTransfers = reinterpret_cast<float*>(dataPtr + pos);

    pos = segmentHeader->bricks.bytePos;
    bricks = reinterpret_cast<Brick*>(dataPtr + pos);

    pos = segmentHeader->brickOrder.bytePos;
    brickOrder = reinterpret_cast<uint32_t*>(dataPtr + pos);

    pos = segmentHeader->nodes.bytePos;
    nodes = reinterpret_cast<DynamicNode*>(dataPtr + pos);

    dataPtr = static_cast<uint8_t*>(segmentData.itemData);
    pos = segmentHeader->synapseSections.bytePos;
    synapseSections = reinterpret_cast<SynapseSection*>(dataPtr + pos);
}

/**
 * @brief allocate memory for the segment
 *
 * @param header header with the size-information
 */
void
DynamicSegment::allocateSegment(SegmentHeader &header)
{
    segmentData.initBuffer<SynapseSection>(header.synapseSections.count, header.staticDataSize);
    segmentData.deleteAll();
}

/**
 * @brief init buffer to avoid undefined values
 */
void
DynamicSegment::initDefaultValues()
{
    // init header and metadata
    dynamicSegmentSettings[0] = DynamicSegmentSettings();

    // init bricks;
    for(uint32_t i = 0; i < segmentHeader->bricks.count; i++) {
        bricks[i] = Brick();
    }

    // init brick-order
    for(uint32_t i = 0; i < segmentHeader->bricks.count; i++) {
        brickOrder[i] = i;
    }

    // init nodes
    for(uint32_t i = 0; i < segmentHeader->nodes.count; i++) {
        nodes[i] = DynamicNode();
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
DynamicSegment::createNewBrick(const JsonItem &brickDef, const uint32_t id)
{
    Brick newBrick;

    // copy metadata
    newBrick.brickId = id;
    if(brickDef.get("type").getString() == "output") {
        newBrick.isOutputBrick = true;
    }
    if(brickDef.get("type").getString() == "transaction") {
        newBrick.isTransactionBrick = true;
    }
    if(brickDef.get("type").getString() == "input") {
        newBrick.isInputBrick = true;
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
DynamicSegment::addBricksToSegment(const JsonItem &metaBase)
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
 * @brief connect a single side of a specific brick
 *
 * @param sourceBrick pointer to the brick
 * @param side side of the brick to connect
 */
void
DynamicSegment::connectBrick(Brick* sourceBrick,
                             const uint8_t side)
{
    const Position next = getNeighborPos(sourceBrick->brickPos, side);
    // debug-output
    // std::cout<<next.x<<" : "<<next.y<<" : "<<next.z<<std::endl;

    if(next.isValid())
    {
        for(uint32_t t = 0; t < segmentHeader->bricks.count; t++)
        {
            Brick* targetBrick = &bricks[t];
            if(targetBrick->brickPos == next)
            {
                sourceBrick->neighbors[side] = targetBrick->brickId;
                targetBrick->neighbors[11 - side] = sourceBrick->brickId;
            }
        }
    }
}

/**
 * @brief connect all breaks of the segment
 */
void
DynamicSegment::connectAllBricks()
{
    for(uint32_t i = 0; i < segmentHeader->bricks.count; i++)
    {
        Brick* sourceBrick = &bricks[i];
        for(uint8_t side = 0; side < 12; side++) {
            connectBrick(sourceBrick, side);
        }
    }
}

/**
 * @brief get next possible brick
 *
 * @param currentBrick actual brick
 * @param maxPathLength maximum path length left
 *
 * @return last brick-id of the gone path
 */
uint32_t
DynamicSegment::goToNextInitBrick(Brick* currentBrick, uint32_t* maxPathLength)
{
    // check path-length to not go too far
    (*maxPathLength)--;
    if(*maxPathLength == 0) {
        return currentBrick->brickId;
    }

    // check based on the chance, if you go to the next, or not
    const float chanceForNext = 0.0f;  // TODO: make hard-coded value configurable
    if(1000.0f * chanceForNext > (rand() % 1000)) {
        return currentBrick->brickId;
    }

    // get a random possible next brick
    const uint8_t possibleNextSides[7] = {9, 3, 1, 4, 11, 5, 2};
    const uint8_t startSide = possibleNextSides[rand() % 7];
    for(uint32_t i = 0; i < 7; i++)
    {
        const uint8_t side = possibleNextSides[(i + startSide) % 7];
        const uint32_t nextBrickId = currentBrick->neighbors[side];
        if(nextBrickId != UNINIT_STATE_32) {
            return goToNextInitBrick(&bricks[nextBrickId], maxPathLength);
        }
    }

    // if no further next brick was found, the give back tha actual one as end of the path
    return currentBrick->brickId;
}

/**
 * @brief init target-brick-list of all bricks
 *
 * @return true, if successful, else false
 */
bool
DynamicSegment::initTargetBrickList()
{
    for(uint32_t i = 0; i < segmentHeader->bricks.count; i++)
    {
        Brick* baseBrick = &bricks[i];

        // ignore output- and transaction-bricks, because they only forward to the border-buffer
        // and not to other bricks
        if(baseBrick->isOutputBrick
                || baseBrick->isTransactionBrick)
        {
            continue;
        }

        // test 1000 samples for possible next bricks
        for(uint32_t counter = 0; counter < 1000; counter++)
        {
            uint32_t maxPathLength = 2; // TODO: make configurable
            const uint32_t brickId = goToNextInitBrick(baseBrick, &maxPathLength);
            if(brickId == baseBrick->brickId)
            {
                LOG_WARNING("brick has no next brick and is a dead-end. Brick-ID: "
                            + std::to_string(brickId));
            }
            baseBrick->possibleTargetNodeBrickIds[counter] = brickId;
        }
    }

    return true;
}
