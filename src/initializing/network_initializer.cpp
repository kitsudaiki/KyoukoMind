/**
 *  @file    network_initializer.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#include "network_initializer.h"
#include <kyouko_root.h>
#include <core/objects/brick.h>

#include <initializing/axon_initializer.h>
#include <initializing/file_parser.h>

#include <core/processing/processing_unit_handler.h>
#include <core/processing/methods/message_processing.h>
#include <core/processing/gpu_interface.h>

#include <core/methods/network_segment_methods.h>
#include <core/methods/brick_initializing_methods.h>

namespace KyoukoMind
{

NetworkInitializer::NetworkInitializer()
{

}

/**
 * @brief NetworkInitializer::initNetwork
 * @return
 */
bool
NetworkInitializer::createNewNetwork(const std::string &fileContent)
{
    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(fileContent == "") {
        return false;
    }

    if(parse2dTestfile(fileContent, m_networkMetaStructure) == false) {
        return false;
    }

    NetworkSegment* segment = KyoukoRoot::m_segment;

    // init segment
    const uint32_t numberOfNodeBricks = getNumberOfNodeBricks();
    const uint32_t totalNumberOfNodes = numberOfNodeBricks * NUMBER_OF_NODES_PER_BRICK;

    assert(initNodeBlocks(*segment, totalNumberOfNodes));
    KyoukoRoot::m_queue->setBorder(getNumberOfBricks());

    // init bricks
    addBricks(*segment);
    connectAllBricks(*segment);
    createAxons(*segment, m_networkMetaStructure);

    if(initSynapseSectionBlocks(*segment, MAX_NUMBER_OF_SYNAPSE_SECTIONS) == false) {
        return false;
    }

    if(initTransferBlocks(*segment, totalNumberOfNodes, MAX_NUMBER_OF_SYNAPSE_SECTIONS) == false) {
        return false;
    }

    // init gpu
    if(initializeGpu(*segment, numberOfNodeBricks) == false) {
        return false;
    }

    return true;
}

/**
 * @brief getNumberOfBricks
 * @param m_networkMetaStructure
 * @return
 */
uint32_t
NetworkInitializer::getNumberOfBricks()
{
    uint32_t numberOfBricks = 0;

    for(uint32_t x = 0; x < m_networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < m_networkMetaStructure[x].size(); y++)
        {
            if(m_networkMetaStructure[x][y].type == 3
                    || m_networkMetaStructure[x][y].type == 2)
            {
                numberOfBricks++;
            }
        }
    }

    return numberOfBricks;
}

/**
 * @brief getNumberOfNodeBricks
 * @param m_networkMetaStructure
 * @return
 */
uint32_t
NetworkInitializer::getNumberOfNodeBricks()
{
    uint32_t numberOfNodeBricks = 0;

    for(uint32_t x = 0; x < m_networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < m_networkMetaStructure[x].size(); y++)
        {
            if(m_networkMetaStructure[x][y].type == 3) {
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
NetworkInitializer::addBricks(NetworkSegment &segment)
{
    uint32_t numberOfNodeBricks = 0;
    uint32_t numberOfBricks = 0;

    BrickQueue* queue = KyoukoRoot::m_queue;

    for(uint32_t x = 0; x < m_networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < m_networkMetaStructure[x].size(); y++)
        {
            const uint32_t brickId = numberOfBricks;
            switch(m_networkMetaStructure[x][y].type)
            {
                case 1:
                    break;
                case 2:
                {
                    Brick* newBrick = new Brick(brickId, x, y);
                    initRandValues(*newBrick);

                    m_networkMetaStructure[x][y].brick = newBrick;
                    m_networkMetaStructure[x][y].brickId = brickId;
                    queue->addToQueue(newBrick);

                    segment.bricks.push_back(newBrick);
                    numberOfBricks++;

                    break;
                }
                case 3:
                {
                    //Brick* brick = new Brick(brickId, x, y);
                    Brick* newBrick = new Brick(brickId, x, y);
                    newBrick->nodeBrickId = numberOfNodeBricks;

                    initRandValues(*newBrick);

                    const uint32_t nodePos = numberOfNodeBricks * NUMBER_OF_NODES_PER_BRICK;
                    assert(nodePos < 0x7FFFFFFF);
                    newBrick->nodePos = static_cast<int32_t>(nodePos);

                    m_networkMetaStructure[x][y].brick = newBrick;
                    m_networkMetaStructure[x][y].brickId = brickId;
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
NetworkInitializer::connectAllBricks(NetworkSegment &segment)
{
    for(uint32_t x = 0; x < m_networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < m_networkMetaStructure[x].size(); y++)
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
                        && m_networkMetaStructure[x][y].type != EMPTY_BRICK
                        && m_networkMetaStructure[next.first][next.second].type != EMPTY_BRICK)
                {
                    const uint32_t sourceId = m_networkMetaStructure[x][y].brick->brickId;
                    const uint32_t targetId = m_networkMetaStructure[next.first][next.second].brick->brickId;
                    connectBricks(segment,
                                  sourceId,
                                  side,
                                  targetId);

                    Neighbor* neighbor = &m_networkMetaStructure[x][y].brick->neighbors[side];
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
NetworkInitializer::getDistantToNextNodeBrick(const uint32_t x,
                                              const uint32_t y,
                                              const uint8_t side)
{
    std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

    uint64_t maxDistance = m_networkMetaStructure.size();

    if(maxDistance > MAX_DISTANCE-1) {
        maxDistance = MAX_DISTANCE-1;
    }

    for(uint32_t distance = 1; distance < m_networkMetaStructure.size(); distance++)
    {
        if(m_networkMetaStructure[next.first][next.second].type == static_cast<uint8_t>(EMPTY_BRICK)) {
            return MAX_DISTANCE;
        }

        if(m_networkMetaStructure[next.first][next.second].type == static_cast<uint8_t>(EMPTY_BRICK)) {
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
NetworkInitializer::getNext(const uint32_t x,
                            const uint32_t y,
                            const uint8_t side)
{
    std::pair<uint32_t, uint32_t> result;
    result.first = UNINIT_STATE_32;
    result.second = UNINIT_STATE_32;

    switch (side)
    {
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
