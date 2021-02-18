/**
 * @file        brick.cpp
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

#include "brick.h"

#include <core/objects/segment.h>
#include <core/objects/node.h>
#include <core/objects/global_values.h>

#include <core/objects/monitoring_container.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief Brick::Brick
 */
Brick::Brick()
{
    initNeighborList();
}

/**
 * @brief Brick::Brick
 * @param brickId
 * @param x
 * @param y
 */
Brick::Brick(const uint32_t &brickId,
             const uint32_t x,
             const uint32_t y)
{
    this->brickId = brickId;
    this->brickPos.x = x;
    this->brickPos.y = y;

    updateMonitoringMetadata();
    initNeighborList();
}

/**
 * @brief Brick::Brick
 * @param other
 */
Brick::Brick(const Brick &other)
{
    if(this != &other)
    {
        this->brickId = other.brickId;
        this->nodeBrickId = other.nodeBrickId;

        this->isInputBrick = other.isInputBrick;
        this->isOutputBrick = other.isOutputBrick;

        this->brickPos = other.brickPos;
        this->nodePos = other.nodePos;

        this->nodeActivity = other.nodeActivity;
        this->synapseActivity = other.synapseActivity;
        this->edgeCreateActivity = other.edgeCreateActivity;
        this->edgeDeleteActivity = other.edgeDeleteActivity;
        this->synapseCreateActivity = other.synapseCreateActivity;
        this->synapseDeleteActivity = other.synapseDeleteActivity;

        for(uint32_t i = 0; i < 12; i++) {
            this->neighbors[i] = other.neighbors[i];
        }
    }
}

/**
 * @brief Brick::operator =
 * @param other
 * @return
 */
Brick
&Brick::operator=(const Brick &other)
{
    if(this != &other)
    {
        this->brickId = other.brickId;
        this->nodeBrickId = other.nodeBrickId;

        this->isInputBrick = other.isInputBrick;
        this->isOutputBrick = other.isOutputBrick;

        this->brickPos = other.brickPos;
        this->nodePos = other.nodePos;

        this->nodeActivity = other.nodeActivity;
        this->synapseActivity = other.synapseActivity;
        this->edgeCreateActivity = other.edgeCreateActivity;
        this->edgeDeleteActivity = other.edgeDeleteActivity;
        this->synapseCreateActivity = other.synapseCreateActivity;
        this->synapseDeleteActivity = other.synapseDeleteActivity;

        for(uint32_t i = 0; i < 12; i++) {
            this->neighbors[i] = other.neighbors[i];
        }
    }

    return *this;
}

/**
 * @brief Brick::~Brick
 */
Brick::~Brick() {}

/**
 * @brief Brick::getRandomNeighbor
 * @return
 */
uint32_t
Brick::getRandomNeighbor(const uint32_t location, const bool random)
{
    const uint8_t inputSide = location;
    const PossibleNext next = getPossibleNext(inputSide, random);

    const uint8_t nextSide = next.next[rand() % 5];
    uint32_t nextLocation = neighbors[nextSide];

    if(nextLocation != UNINIT_STATE_32) {
        nextLocation += static_cast<uint32_t>(11 - nextSide) << 24;
    }

    return nextLocation;
}


/**
 * @brief Brick::initNeighborList
 */
void
Brick::initNeighborList()
{
    for(uint8_t i = 0; i < 12; i++) {
        neighbors[i] = UNINIT_STATE_32;
    }
}

/**
 * @brief Brick::getPossibleNext
 * @param inputSide
 * @return
 */
const Brick::PossibleNext
Brick::getPossibleNext(const uint8_t inputSide, const bool random)
{
    PossibleNext next;

    if(random)
    {
        next.next[0] = rand() % 12;
        next.next[1] = rand() % 12;
        next.next[2] = rand() % 12;
        next.next[3] = rand() % 12;
        next.next[4] = rand() % 12;
        return next;
    }

    switch(inputSide)
    {
        case 0:
        {
            next.next[0] = 4;
            next.next[1] = 5;
            next.next[2] = 7;
            next.next[3] = 8;
            next.next[4] = 11;
            break;
        }

        case 1:
        {
            next.next[0] = 5;
            next.next[1] = 6;
            next.next[2] = 7;
            next.next[3] = 8;
            next.next[4] = 10;
            break;
        }

        case 2:
        {
            next.next[0] = 3;
            next.next[1] = 4;
            next.next[2] = 5;
            next.next[3] = 6;
            next.next[4] = 9;
            break;
        }

        case 3:
        {
            next.next[0] = 5;
            next.next[1] = 7;
            next.next[2] = 8;
            next.next[3] = 2;
            next.next[4] = 11;
            break;
        }
        case 4:
        {
            next.next[0] = 6;
            next.next[1] = 7;
            next.next[2] = 8;
            next.next[3] = 2;
            next.next[4] = 10;
            break;
        }
        case 5:
        {
            next.next[0] = 3;
            next.next[1] = 6;
            next.next[2] = 7;
            next.next[3] = 0;
            next.next[4] = 10;
            break;
        }
        case 6:
        {
            next.next[0] = 4;
            next.next[1] = 5;
            next.next[2] = 8;
            next.next[3] = 1;
            next.next[4] = 11;
            break;
        }
        case 7:
        {
            next.next[0] = 3;
            next.next[1] = 4;
            next.next[2] = 5;
            next.next[3] = 1;
            next.next[4] = 9;
            break;
        }
        case 8:
        {
            next.next[0] = 3;
            next.next[1] = 4;
            next.next[2] = 6;
            next.next[3] = 0;
            next.next[4] = 9;
            break;
        }
        case 9:
        {
            next.next[0] = 5;
            next.next[1] = 6;
            next.next[2] = 7;
            next.next[3] = 8;
            next.next[4] = 2;
            break;
        }
        case 10:
        {
            next.next[0] = 3;
            next.next[1] = 4;
            next.next[2] = 5;
            next.next[3] = 8;
            next.next[4] = 1;
            break;
        }
        case 11:
        {
            next.next[0] = 3;
            next.next[1] = 4;
            next.next[2] = 6;
            next.next[3] = 7;
            next.next[4] = 0;
            break;
        }

        default:
        {
            assert(false);
        }
    }

    return next;
}

/**
 * @brief Brick::updateMonitoringMetadata
 */
void
Brick::updateMonitoringMetadata()
{
    KyoukoRoot::monitoringBrickMessage.brickInfos[brickId].xPos = brickPos.x;
    KyoukoRoot::monitoringBrickMessage.brickInfos[brickId].yPos = brickPos.y;
    KyoukoRoot::monitoringBrickMessage.brickInfos[brickId].brickId = brickId;
}
