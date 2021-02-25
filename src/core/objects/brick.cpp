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
    for(uint8_t i = 0; i < 12; i++) {
        neighbors[i] = UNINIT_STATE_32;
    }
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
