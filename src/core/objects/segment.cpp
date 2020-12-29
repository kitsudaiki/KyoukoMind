/**
 * @file        segment.cpp
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

#include "segment.h"

#include <core/objects/transfer_objects.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

#include <initializing/segment_initializing.h>

Segment::Segment()
{
    initGlobalValues(*this);
}

/**
 * @brief BrickHandler::getMetadata
 * @return
 */
DataItem*
Segment::getMetadata()
{
    DataArray* edges = new DataArray();
    DataArray* nodes = new DataArray();

    // collect data
    for(uint32_t i = 0; i < bricks.itemCapacity; i++)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[i];

        if(brick->nodePos != UNINIT_STATE_32) {
            nodes->append(new DataValue(static_cast<long>(brick->brickId)));
        }

        edges->append(new DataValue(static_cast<long>(brick->brickId)));
    }

    // build result
    DataMap* result = new DataMap();
    result->insert("edges", edges);
    result->insert("nodes", nodes);

    return result;
}

/**
 * @brief BrickHandler::connect
 * @param sourceBrickId
 * @param sourceSide
 * @param targetBrickId
 * @return
 */
bool
Segment::connectBricks(const uint32_t sourceBrickId,
                              const uint8_t sourceSide,
                              const uint32_t targetBrickId)
{
    Brick* sourceBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[sourceBrickId];
    Brick* targetBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[targetBrickId];
    return sourceBrick->connectBricks(sourceSide, *targetBrick);
}

/**
 * disconnect two bricks from the handler from each other
 *
 * @return result of the sub-call
 */
bool
Segment::disconnectBricks(const uint32_t sourceBrickId,
                                 const uint8_t sourceSide)
{
    Brick* sourceBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[sourceBrickId];
    return sourceBrick->disconnectBricks(sourceSide);
}
