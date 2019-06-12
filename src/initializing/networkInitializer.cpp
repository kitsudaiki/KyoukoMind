/**
 *  @file    networkInitializer.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "networkInitializer.h"
#include <kyoukoNetwork.h>
#include <core/bricks/brickObjects/brick.h>

#include <initializing/axonInitializer.h>
#include <initializing/fileParser.h>

#include <core/bricks/brickHandler.h>
#include <core/processing/processingUnitHandler.h>

#include <core/bricks/brickObjects/brick.h>
#include <core/bricks/brickMethods/commonBrickMethods.h>
#include <core/bricks/brickMethods/bufferControlMethods.h>
#include <core/processing/processingMethods/messageProcessingMethods.h>

namespace KyoukoMind
{

/**
 * @brief NetworkInitializer::initNetwork
 * @return
 */
bool
createNewNetwork(const std::string fileContent)
{
    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(fileContent == "")
    {
        return false;
    }

    InitStructure networkMetaStructure;

    //std::string fileContent = readFile(filePath);

    if(parse2dTestfile(fileContent, &networkMetaStructure) == false) {
        return false;
    }

    addBricks(NUMBER_OF_NODES_PER_BRICK, &networkMetaStructure);
    connectAllBricks(&networkMetaStructure);
    createAxons(&networkMetaStructure);

    return true;
}

/**
 * @brief BrickInitilizer::addBrick
 * @param x
 * @param y
 * @param nodeNumberPerBrick
 * @return
 */
void
addBricks(const uint32_t nodeNumberPerBrick,
          InitStructure* networkMetaStructure)
{
    for(uint32_t x = 0; x < (*networkMetaStructure).size(); x++)
    {
        for(uint32_t y = 0; y < (*networkMetaStructure)[x].size(); y++)
        {
            switch((*networkMetaStructure)[x][y].type)
            {
                case 1:
                    break;
                case 2:
                {
                    Brick* brick = new Brick((*networkMetaStructure)[x][y].brickId, x, y);
                    (*networkMetaStructure)[x][y].brick = brick;
                    KyoukoNetwork::m_brickHandler->addBrick((*networkMetaStructure)[x][y].brickId, brick);
                    break;
                }
                case 3:
                {
                    Brick* brick = new Brick((*networkMetaStructure)[x][y].brickId, x, y);
                    initNodeBlocks(brick, nodeNumberPerBrick);
                    initEdgeSectionBlocks(brick, 0);
                    (*networkMetaStructure)[x][y].brick = brick;
                    KyoukoNetwork::m_brickHandler->addBrick((*networkMetaStructure)[x][y].brickId, brick);
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
 * @brief BrickInitilizer::createNetwork
 * @return
 */
bool
connectAllBricks(InitStructure *networkMetaStructure)
{
    for(uint32_t x = 0; x < (*networkMetaStructure).size(); x++)
    {
        for(uint32_t y = 0; y < (*networkMetaStructure)[x].size(); y++)
        {
            std::vector<uint8_t> sideOrder = {9,10,11,14,13,12};
            for(uint8_t i = 0; i < sideOrder.size(); i++)
            {
                uint8_t side = sideOrder[i];
                // get new neighbor
                std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

                // set the values in the neighbor-struct
                if(next.first != UNINIT_STATE_32 && next.second != UNINIT_STATE_32
                         && (*networkMetaStructure)[x][y].type != EMPTY_BRICK
                         && (*networkMetaStructure)[next.first][next.second].type != EMPTY_BRICK)
                {
                    KyoukoNetwork::m_brickHandler->connect(
                                (*networkMetaStructure)[x][y].brick->brickId,
                                side,
                                 (*networkMetaStructure)[next.first][next.second].brickId);

                    (*networkMetaStructure)[x][y].brick->neighbors[side].targetBrickPos.x = next.first;
                    (*networkMetaStructure)[x][y].brick->neighbors[side].targetBrickPos.y = next.second;

                }
            }
        }
    }
    return true;
}

/**
 * @brief NetworkInitializer::getDistantToNextNodeBrick
 * @param x
 * @param y
 * @param side
 * @return
 */
uint32_t
getDistantToNextNodeBrick(const uint32_t x,
                          const uint32_t y,
                          const uint8_t side,
                          InitStructure* networkMetaStructure)
{
    std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

    uint32_t maxDistance = (*networkMetaStructure).size();

    if(maxDistance > MAX_DISTANCE-1) {
        maxDistance = MAX_DISTANCE-1;
    }

    for(uint32_t distance = 1; distance < (*networkMetaStructure).size(); distance++)
    {
        if((*networkMetaStructure)[next.first][next.second].type == (uint8_t)EMPTY_BRICK) {
            return MAX_DISTANCE;
        }
        if((*networkMetaStructure)[next.first][next.second].type == (uint8_t)NODE_BRICK) {
            return distance;
        }

        next = getNext(next.first, next.second, side);
    }
    return MAX_DISTANCE;
}

/**
 * @brief BrickInitilizer::getNext
 * @param x
 * @param y
 * @param side
 * @return
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

}
