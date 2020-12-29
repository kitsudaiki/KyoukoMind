/**
 * @file        network_initializer.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2018 Tobias Anker
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

#include "network_initializer.h"
#include <kyouko_root.h>
#include <core/objects/segment.h>
#include <core/objects/node.h>
#include <core/objects/global_values.h>

#include <initializing/axon_initializer.h>
#include <initializing/file_parser.h>

#include <core/processing/processing_unit_handler.h>
#include <core/processing/gpu/gpu_processing_uint.h>
#include <initializing/segment_initializing.h>

/**
 * @brief constructor
 */
NetworkInitializer::NetworkInitializer() {}

/**
 * @brief create blank new network
 *
 * @param fileContent file to parse with the basic structure of the network
 *
 * @return true, if successfull, else false
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

    const uint32_t numberOfBricks = parse2dTestfile(fileContent, m_netMetaStructure);

    // update message for the monitoring
    KyoukoRoot::monitoringBrickMessage.numberOfInfos = numberOfBricks;
    if(numberOfBricks == 0) {
        return false;
    }


    // init segment
    Segment* segment = KyoukoRoot::m_segment;
    const uint32_t numberOfNodeBricks = getNumberOfNodeBricks();
    GlobalValues* globalValues = getBuffer<GlobalValues>(segment->globalValues);
    const uint32_t totalNumberOfNodes = numberOfNodeBricks * globalValues->numberOfNodesPerBrick;

    if(initNodeBlocks(*segment, totalNumberOfNodes) == false) {
        return false;
    }

    if(initRandomValues(*segment) == false) {
        return false;
    }

    if(initEdgeSectionBlocks(*segment, totalNumberOfNodes) == false) {
        return false;
    }

    // init bricks
    initBricks(*segment, numberOfBricks);
    addBricks(*segment);
    connectAllBricks(*segment);

    // init axons
    createAxons(*segment, m_netMetaStructure);

    // init synapses
    if(initSynapseSectionBlocks(*segment, MAX_NUMBER_OF_SYNAPSE_SECTIONS) == false) {
        return false;
    }

    // mark all synapses als delted to make them usable
    if(segment->synapses.deleteAll() == false) {
        return false;
    }

    // init buffer for data-transfer between host and gpu
    if(initTransferBlocks(*segment, totalNumberOfNodes, MAX_NUMBER_OF_SYNAPSE_SECTIONS) == false) {
        return false;
    }

    return true;
}

/**
 * @brief get number of node-bricks
 *
 * @return number of node-bricks
 */
uint32_t
NetworkInitializer::getNumberOfNodeBricks()
{
    uint32_t numberOfNodeBricks = 0;

    for(uint32_t x = 0; x < m_netMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < m_netMetaStructure[x].size(); y++)
        {
            if(m_netMetaStructure[x][y].type == NODE_BRICK
                    || m_netMetaStructure[x][y].type == OUTPUT_NODE_BRICK)
            {
                numberOfNodeBricks++;
            }
        }
    }

    return numberOfNodeBricks;
}

/**
 * @brief add new brick
 *
 * @param segment segment where to add a new brick
 */
void
NetworkInitializer::addBricks(Segment &segment)
{
    uint32_t numberOfNodeBricks = 0;
    uint32_t numberOfBricks = 0;
    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    for(uint32_t x = 0; x < m_netMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < m_netMetaStructure[x].size(); y++)
        {
            const uint32_t brickId = numberOfBricks;
            switch(m_netMetaStructure[x][y].type)
            {
                case 1:
                    break;
                case 2:
                {
                    Brick newBrick(brickId, x, y);
                    newBrick.nodeBrickId = UNINIT_STATE_32;

                    // segment.bricks.addNewItem(newBrick, true);
                    getBuffer<Brick>(segment.bricks)[numberOfBricks] = newBrick;

                    Brick* ptr = &getBuffer<Brick>(segment.bricks)[numberOfBricks];
                    m_netMetaStructure[x][y].brick = ptr;
                    numberOfBricks++;

                    break;
                }
                case 3:
                {
                    //Brick* brick = new Brick(brickId, x, y);
                    Brick newBrick(brickId, x, y);
                    newBrick.nodeBrickId = numberOfNodeBricks;

                    const uint32_t nodePos = numberOfNodeBricks * globalValues->numberOfNodesPerBrick;
                    assert(nodePos < 0x7FFFFFFF);
                    newBrick.nodePos = nodePos;

                    // segment.bricks.addNewItem(newBrick, true);
                    getBuffer<Brick>(segment.bricks)[numberOfBricks] = newBrick;
                    Brick* ptr = &getBuffer<Brick>(segment.bricks)[numberOfBricks];
                    m_netMetaStructure[x][y].brick = ptr;

                    numberOfBricks++;
                    numberOfNodeBricks++;

                    break;
                }
                case 4:
                {
                    //Brick* brick = new Brick(brickId, x, y);
                    Brick newBrick(brickId, x, y);
                    newBrick.nodeBrickId = numberOfNodeBricks;
                    newBrick.isOutputBrick = true;

                    const uint32_t nodePos = numberOfNodeBricks * globalValues->numberOfNodesPerBrick;
                    assert(nodePos < 0x7FFFFFFF);
                    newBrick.nodePos = nodePos;

                    Node* array = getBuffer<Node>(segment.nodes);
                    for(uint32_t i = 0; i < globalValues->numberOfNodesPerBrick; i++)
                    {
                        array[i + nodePos].border = -1.0f;
                    }

                    // segment.bricks.addNewItem(newBrick, true);
                    getBuffer<Brick>(segment.bricks)[numberOfBricks] = newBrick;
                    Brick* ptr = &getBuffer<Brick>(segment.bricks)[numberOfBricks];
                    m_netMetaStructure[x][y].brick = ptr;

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
 *
 * @param segment segment with the bricks to connect
 */
void
NetworkInitializer::connectAllBricks(Segment &segment)
{
    for(uint32_t x = 0; x < m_netMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < m_netMetaStructure[x].size(); y++)
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
                        && m_netMetaStructure[x][y].type != EMPTY_BRICK
                        && m_netMetaStructure[next.first][next.second].type != EMPTY_BRICK)
                {
                    const uint32_t sourceId = m_netMetaStructure[x][y].brick->brickId;
                    const uint32_t targetId = m_netMetaStructure[next.first][next.second].brick->brickId;

                    assert(targetId != UNINIT_STATE_32);
                    segment.connectBricks(sourceId, side, targetId);

                    Brick* targetBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[targetId];
                    targetBrick->brickPos.x = next.first;
                    targetBrick->brickPos.y = next.second;
                }
            }
        }
    }

    return;
}

/**
 * @brief calculate the distance to the next node-brick from a specific brick in a direction
 *
 * @param x actual x-position
 * @param y actual x-position
 * @param side side to go to next
 *
 * @return number of bricks to the next node-brick
 */
uint32_t
NetworkInitializer::getDistantToNextNodeBrick(const uint32_t x,
                                              const uint32_t y,
                                              const uint8_t side)
{
    std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

    uint64_t maxDistance = m_netMetaStructure.size();

    if(maxDistance > MAX_DISTANCE-1) {
        maxDistance = MAX_DISTANCE-1;
    }

    for(uint32_t distance = 1; distance < m_netMetaStructure.size(); distance++)
    {
        if(m_netMetaStructure[next.first][next.second].type == static_cast<uint8_t>(EMPTY_BRICK)) {
            return MAX_DISTANCE;
        }

        if(m_netMetaStructure[next.first][next.second].type == static_cast<uint8_t>(EMPTY_BRICK)) {
            return distance;
        }

        next = getNext(next.first, next.second, side);
    }

    return MAX_DISTANCE;
}

/**
 * @brief get the next position in the raster for a specific brick and side
 *
 * @param x actual x-position
 * @param y actual x-position
 * @param side side to go to next
 *
 * @return pair next x-y-coordinates
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
