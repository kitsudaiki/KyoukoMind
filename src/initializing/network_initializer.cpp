/**
 *  @file    network_initializer.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "network_initializer.h"
#include <root_object.h>
#include <core/objects/brick.h>

#include <initializing/axon_initializer.h>
#include <initializing/file_parser.h>

#include <core/brick_handler.h>
#include <core/processing/processing_unit_handler.h>
#include <core/processing/processing_methods/message_processing_methods.h>
#include <core/processing/processing_methods/brick_initializing_methods.h>

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
    if(fileContent == "") {
        return false;
    }

    InitStructure networkMetaStructure;
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
            const BrickID brickId = (*networkMetaStructure)[x][y].brickId;
            switch((*networkMetaStructure)[x][y].type)
            {
                case 1:
                    break;
                case 2:
                {
                    Brick* brick = new Brick(brickId, x, y);
                    (*networkMetaStructure)[x][y].brick = brick;
                    RootObject::m_brickHandler->addBrick(brickId, brick);
                    break;
                }
                case 3:
                {
                    Brick* brick = new Brick(brickId, x, y);
                    initNodeBlocks(*brick, nodeNumberPerBrick);
                    initSynapseSectionBlocks(*brick, 0);
                    (*networkMetaStructure)[x][y].brick = brick;
                    RootObject::m_brickHandler->addBrick(brickId, brick);
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
connectAllBricks(InitStructure *metaStructure)
{
    for(uint32_t x = 0; x < (*metaStructure).size(); x++)
    {
        for(uint32_t y = 0; y < (*metaStructure)[x].size(); y++)
        {
            std::vector<uint8_t> sideOrder = {9,10,11,14,13,12};
            for(uint8_t i = 0; i < sideOrder.size(); i++)
            {
                uint8_t side = sideOrder[i];
                // get new neighbor
                std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

                // set the values in the neighbor-struct
                if(next.first != UNINIT_STATE_32 && next.second != UNINIT_STATE_32
                         && (*metaStructure)[x][y].type != EMPTY_BRICK
                         && (*metaStructure)[next.first][next.second].type != EMPTY_BRICK)
                {
                    const BrickID sourceId = (*metaStructure)[x][y].brick->brickId;
                    const BrickID targetId = (*metaStructure)[next.first][next.second].brickId;
                    RootObject::m_brickHandler->connect(sourceId,
                                                           side,
                                                           targetId);

                    Neighbor* neighbor = &(*metaStructure)[x][y].brick->neighbors[side];
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
                          InitStructure* metaStructure)
{
    std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

    uint32_t maxDistance = (*metaStructure).size();

    if(maxDistance > MAX_DISTANCE-1) {
        maxDistance = MAX_DISTANCE-1;
    }

    for(uint32_t distance = 1; distance < (*metaStructure).size(); distance++)
    {
        if((*metaStructure)[next.first][next.second].type == (uint8_t)EMPTY_BRICK) {
            return MAX_DISTANCE;
        }
        if((*metaStructure)[next.first][next.second].type == (uint8_t)NODE_BRICK) {
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
