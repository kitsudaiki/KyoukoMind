/**
 *  @file    network_initializer.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#include "network_initializer.h"
#include <root_object.h>
#include <core/objects/brick.h>

#include <initializing/axon_initializer.h>
#include <initializing/file_parser.h>

#include <core/processing/processing_unit_handler.h>
#include <core/processing/edge_container_processing.h>
#include <core/methods/brick_initializing_methods.h>

namespace KyoukoMind
{

/**
 * @brief NetworkInitializer::initNetwork
 * @return
 */
bool
createNewNetwork(const std::string &fileContent)
{
    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(fileContent == "") {
        return false;
    }

    std::vector<std::vector<InitMetaDataEntry>> networkMetaStructure;
    if(parse2dTestfile(fileContent, networkMetaStructure) == false) {
        return false;
    }

    NetworkSegment* segment = RootObject::m_segment;

    // init segment
    assert(initSynapseSectionBlocks(*segment, 1));
    const uint32_t totalNumberOfNodes = getNumberOfNodeBricks(networkMetaStructure)
                                        * NUMBER_OF_NODES_PER_BRICK;
    assert(initNodeBlocks(*segment, totalNumberOfNodes));
    RootObject::m_queue->setBorder(getNumberOfBricks(networkMetaStructure));

    // init bricks
    addBricks(*segment, networkMetaStructure);
    connectAllBricks(*segment, networkMetaStructure);
    createAxons(*segment, networkMetaStructure);

    return true;
}

/**
 * @brief getNumberOfBricks
 * @param networkMetaStructure
 * @return
 */
uint32_t
getNumberOfBricks(std::vector<std::vector<InitMetaDataEntry>> &networkMetaStructure)
{
    uint32_t numberOfBricks = 0;

    for(uint32_t x = 0; x < networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < networkMetaStructure[x].size(); y++)
        {
            if(networkMetaStructure[x][y].type == 3
                    || networkMetaStructure[x][y].type == 2)
            {
                numberOfBricks++;
            }
        }
    }

    return numberOfBricks;
}

/**
 * @brief getNumberOfNodeBricks
 * @param networkMetaStructure
 * @return
 */
uint32_t
getNumberOfNodeBricks(std::vector<std::vector<InitMetaDataEntry>> &networkMetaStructure)
{
    uint32_t numberOfNodeBricks = 0;

    for(uint32_t x = 0; x < networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < networkMetaStructure[x].size(); y++)
        {
            if(networkMetaStructure[x][y].type == 3) {
                numberOfNodeBricks++;
            }
        }
    }

    return numberOfNodeBricks;
}

/**
 * @brief BrickInitilizer::addBrick
 * @param x
 * @param y
 * @param nodeNumberPerBrick
 * @return
 */
void
addBricks(NetworkSegment &segment,
          std::vector<std::vector<InitMetaDataEntry>> &networkMetaStructure)
{
    uint32_t numberOfNodeBricks = 0;
    uint32_t numberOfBricks = 0;

    BrickQueue* queue = RootObject::m_queue;

    for(uint32_t x = 0; x < networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < networkMetaStructure[x].size(); y++)
        {
            const BrickID brickId = numberOfBricks;
            switch(networkMetaStructure[x][y].type)
            {
                case 1:
                    break;
                case 2:
                {
                    Brick* newBrick = new Brick(brickId, x, y);
                    initRandValues(*newBrick);

                    networkMetaStructure[x][y].brick = newBrick;
                    networkMetaStructure[x][y].brickId = brickId;
                    queue->addToQueue(newBrick);

                    segment.bricks.push_back(newBrick);
                    numberOfBricks++;

                    break;
                }
                case 3:
                {
                    //Brick* brick = new Brick(brickId, x, y);
                    Brick* newBrick = new Brick(brickId, x, y);
                    initRandValues(*newBrick);

                    const uint32_t nodePos = numberOfNodeBricks * NUMBER_OF_NODES_PER_BRICK;
                    assert(nodePos < 0x7FFFFFFF);
                    newBrick->nodePos = static_cast<int32_t>(nodePos);

                    networkMetaStructure[x][y].brick = newBrick;
                    networkMetaStructure[x][y].brickId = brickId;
                    queue->addToQueue(newBrick);

                    segment.bricks.push_back(newBrick);
                    numberOfBricks++;
                    numberOfNodeBricks++;

                    break;
                }
                default:
                    break;
            }
        }
    }

    return;
}

/**
 * connect all brickts of the initializing data with each other
 */
void
connectAllBricks(NetworkSegment &segment,
                 std::vector<std::vector<InitMetaDataEntry>> &metaStructure)
{
    for(uint32_t x = 0; x < metaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < metaStructure[x].size(); y++)
        {
            std::vector<uint8_t> sideOrder = {9,10,11,14,13,12};
            for(uint8_t i = 0; i < sideOrder.size(); i++)
            {
                uint8_t side = sideOrder[i];
                // get new neighbor
                std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

                // set the values in the neighbor-struct
                if(next.first != UNINIT_STATE_32
                        && next.second != UNINIT_STATE_32
                        && metaStructure[x][y].type != EMPTY_BRICK
                        && metaStructure[next.first][next.second].type != EMPTY_BRICK)
                {
                    const BrickID sourceId = metaStructure[x][y].brick->brickId;
                    const BrickID targetId = metaStructure[next.first][next.second].brick->brickId;
                    connectBricks(segment,
                                  sourceId,
                                  side,
                                  targetId);
                    Neighbor* neighbor = &metaStructure[x][y].brick->neighbors[side];
                    neighbor->targetBrickPos.x = next.first;
                    neighbor->targetBrickPos.y = next.second;
                }
            }
        }
    }

    return;
}

/**
 * calculate the distance to the next node-brick from a specific brick in a specific direction
 *
 * @return number of bricks to the next node-brick
 */
uint32_t
getDistantToNextNodeBrick(const uint32_t x,
                          const uint32_t y,
                          const uint8_t side,
                          std::vector<std::vector<InitMetaDataEntry>> &metaStructure)
{
    std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

    uint64_t maxDistance = metaStructure.size();

    if(maxDistance > MAX_DISTANCE-1) {
        maxDistance = MAX_DISTANCE-1;
    }

    for(uint32_t distance = 1; distance < metaStructure.size(); distance++)
    {
        if(metaStructure[next.first][next.second].type == static_cast<uint8_t>(EMPTY_BRICK)) {
            return MAX_DISTANCE;
        }

        if(metaStructure[next.first][next.second].type == static_cast<uint8_t>(EMPTY_BRICK)) {
            return distance;
        }

        next = getNext(next.first, next.second, side);
    }
    return MAX_DISTANCE;
}

/**
 * get the next position in the raster for a specific node and side
 *
 * @return pair of the x-y-coordinates of the next node
 */
std::pair<uint32_t, uint32_t>
getNext(const uint32_t x,
        const uint32_t y,
        const uint8_t side)
{
    std::pair<uint32_t, uint32_t> result;
    result.first = UNINIT_STATE_32;
    result.second = UNINIT_STATE_32;

    switch (side) {
    case 9:
        {
            result.first = x - 1;
            if(x % 2 == 1) {
                result.second = y + 1;
            } else {
                result.second = y;
            }
            break;
        }
    case 10:
        {
            result.first = x;
            result.second = y + 1;
            break;
        }
    case 11:
        {
            result.first = x + 1;
            if(x % 2 == 1) {
                result.second = y + 1;
            } else {
                result.second = y;
            }
            break;
        }
    case 14:
        {
            result.first = x + 1;
            if(x % 2 == 1) {
                result.second = y;
            } else {
                result.second = y - 1;
            }
            break;
        }
    case 13:
        {
            result.first = x;
            result.second = y - 1;
            break;
        }
    case 12:
        {
            result.first = x - 1;
            if(x % 2 == 1) {
                result.second = y;
            } else {
                result.second = y - 1;
            }
            break;
        }
    default:
        break;
    }
    return result;
}

} // namespace KyoukoMind
