#include "dynamic_segment.h"

DynamicSegment::DynamicSegment(const uint32_t numberOfBricks,
                               const uint32_t numberOfNodes,
                               const uint64_t numberOfSynapseSections)
    : AbstractSegment()
{
    m_type = DYNAMIC_SEGMENT;

    SegmentHeader header = createNewHeader(numberOfBricks,
                                           numberOfNodes,
                                           numberOfSynapseSections);
    allocateSegment(header);
    initSegmentPointer(header);
    initDefaultValues(numberOfBricks, numberOfNodes);
}

DynamicSegment::~DynamicSegment()
{

}

SegmentHeader
DynamicSegment::createNewHeader(const uint32_t numberOfBricks,
                                const uint32_t numberOfNodes,
                                const uint64_t numberOfSynapseSections)
{
    SegmentHeader segmentHeader;
    uint32_t segmentDataPos = 0;

    // init header
    segmentDataPos += 1 * sizeof(SegmentHeader);

    // init settings
    segmentHeader.settings.count = 1;
    segmentHeader.settings.bytePos = segmentDataPos;
    segmentDataPos += 1 * sizeof(SegmentSettings);

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
    segmentDataPos += numberOfNodes * sizeof(Node);

    segmentHeader.staticDataSize = segmentDataPos;

    // init synapse sections
    segmentDataPos = 0;
    segmentHeader.synapseSections.count = numberOfSynapseSections;
    segmentHeader.synapseSections.bytePos = segmentDataPos;

    return segmentHeader;
}

/**
 * @brief initSegmentPointer
 * @param segment
 * @param header
 */
void
DynamicSegment::initSegmentPointer(const SegmentHeader &header)
{
    uint8_t* dataPtr = static_cast<uint8_t*>(segmentData.staticData);
    uint64_t pos = 0;

    segmentHeader = reinterpret_cast<SegmentHeader*>(dataPtr + pos);
    segmentHeader[0] = header;

    pos = 256;
    segmentSettings = reinterpret_cast<SegmentSettings*>(dataPtr + pos);
    pos = segmentHeader->bricks.bytePos;
    bricks = reinterpret_cast<Brick*>(dataPtr + pos);
    pos = segmentHeader->brickOrder.bytePos;
    brickOrder = reinterpret_cast<uint32_t*>(dataPtr + pos);
    pos = segmentHeader->nodes.bytePos;
    nodes = reinterpret_cast<Node*>(dataPtr + pos);

    dataPtr = static_cast<uint8_t*>(segmentData.itemData);
    pos = segmentHeader->synapseSections.bytePos;
    synapseSections = reinterpret_cast<SynapseSection*>(dataPtr + pos);
}

/**
 * @brief allocateSegment
 * @param header
 * @return
 */
void
DynamicSegment::allocateSegment(SegmentHeader &header)
{
    segmentData.initBuffer<SynapseSection>(header.synapseSections.count, header.staticDataSize);
    segmentData.deleteAll();
}

/**
 * @brief DynamicSegment::initDefaultValues
 * @param numberOfBricks
 * @param numberOfNodes
 */
void
DynamicSegment::initDefaultValues(const uint32_t numberOfBricks,
                                  const uint32_t numberOfNodes)
{
    // init header and metadata
    segmentSettings[0] = SegmentSettings();

    // init bricks;
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        bricks[i] = Brick();
    }

    // init brick-order
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        brickOrder[i] = i;
    }

    // init nodes
    for(uint32_t i = 0; i < numberOfNodes; i++) {
        nodes[i] = Node();
    }
}

Brick
DynamicSegment::createNewBrick(const JsonItem &brickDef, const uint32_t id)
{
    Brick newBrick;

    // copy metadata
    newBrick.brickId = id;
    newBrick.nodeBrickId = id;
    if(brickDef.contains("type"))
    {
        if(brickDef.get("type").getString() == "output") {
            newBrick.isOutputBrick = true;
        }
        if(brickDef.get("type").getString() == "input") {
            newBrick.isInputBrick = true;
        }
    }

    // copy position
    newBrick.brickPos.x = brickDef.get("x").getInt();
    newBrick.brickPos.y = brickDef.get("y").getInt();
    newBrick.brickPos.z = brickDef.get("z").getInt();

    // set other values
    newBrick.numberOfNodes = brickDef.get("number_of_nodes").getInt();

    return newBrick;
}

/**
 * @brief addBricksToSegment
 * @param segment
 * @param initMetaData
 * @param metaBase
 */
void
DynamicSegment::addBricksToSegment(const JsonItem &metaBase)
{
    uint32_t nodeBrickIdCounter = 0;
    uint32_t nodePosCounter = 0;
    JsonItem brickDef = metaBase.get("bricks");

    for(uint32_t i = 0; i < brickDef.size(); i++)
    {
        Brick newBrick = createNewBrick(brickDef[i], i);

        // handle node-brick
        newBrick.nodePos = nodePosCounter;

        for(uint32_t j = 0; j < newBrick.numberOfNodes; j++) {
            nodes[j + nodePosCounter].nodeBrickId = newBrick.nodeBrickId;
        }

        // copy new brick to segment
        bricks[nodeBrickIdCounter] = newBrick;
        assert(nodeBrickIdCounter == newBrick.nodeBrickId);
        nodeBrickIdCounter++;
        nodePosCounter += newBrick.numberOfNodes;
    }

    return;
}

/**
 * @brief connectBrick
 * @param segment
 * @param sourceBrick
 * @param side
 */
void
DynamicSegment::connectBrick(Brick* sourceBrick,
                             const uint8_t side)
{
    sourceBrick->neighbors[side] = UNINIT_STATE_32;
    Position next = getNeighborPos(sourceBrick->brickPos, side);
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
    else
    {
        // TODO: connect to output-transfer
    }
}

/**
 * @brief connect all bricks in the parser-output based on its coordinates to identify neighbors
 *
 * @param parserOutput output coming from the parser
 * @param x current x-position
 * @param y current y-position
 * @param z current z-position
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
 * @brief get the next position in the raster for a specific brick and side
 *
 * @param x current x-position
 * @param y current y-position
 * @param z current z-position
 * @param side side to go to next
 *
 * @return position of the next brick for the specific side
 */
Position
DynamicSegment::getNeighborPos(Position sourcePos, const uint8_t side)
{
    Position result;
    result.x = UNINIT_STATE_32;
    result.y = UNINIT_STATE_32;
    result.z = UNINIT_STATE_32;

    switch (side)
    {
    case 0:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z - 1;
            break;
        }
    case 1:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z - 1;
            break;
        }
    case 2:
        {
            result.x = sourcePos.x;
            result.y = sourcePos.y;
            result.z = sourcePos.z - 1;
            break;
        }
    case 3:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z;
            break;
        }
    case 4:
        {
            result.x = sourcePos.x + 1;
            result.y = sourcePos.y;
            result.z = sourcePos.z;
            break;
        }
    case 5:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z;
            break;
        }
    case 8:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z;
            break;
        }
    case 7:
        {
            result.x = sourcePos.x - 1;
            result.y = sourcePos.y;
            result.z = sourcePos.z;
            break;
        }
    case 6:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z;
            break;
        }
    case 9:
        {
            result.x = sourcePos.x;
            result.y = sourcePos.y;
            result.z = sourcePos.z + 1;
            break;
        }
    case 10:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z + 1;
            break;
        }
    case 11:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z + 1;
            break;
        }
    default:
        assert(false);
    }
    return result;
}

/**
 * @brief FanBrickInitializer::initTargetBrickList
 * @param segment
 * @return
 */
bool
DynamicSegment::initTargetBrickList()
{
    Brick* bricks = bricks;

    // iterate over all bricks
    for(uint32_t i = 0; i < segmentHeader->bricks.count; i++)
    {
        Brick* baseBrick = &bricks[i];
        if(baseBrick->isOutputBrick != 0) {
            continue;
        }

        // get 1000 samples
        uint32_t counter = 0;
        while(counter < 1000)
        {
            baseBrick->possibleTargetNodeBrickIds[counter] = baseBrick->brickId + 1;
            counter++;
            /*uint8_t nextSide = getPossibleNext();
            const uint32_t nextBrickId = baseBrick->neighbors[nextSide];
            if(nextBrickId != UNINIT_STATE_32)
            {
                baseBrick->possibleTargetNodeBrickIds[counter] = nextBrickId;
                counter++;
                if(counter >= 1000) {
                    break;
                }
            }*/
        }
        assert(counter == 1000);
    }

    return true;
}
