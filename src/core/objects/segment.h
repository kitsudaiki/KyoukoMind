/**
 * @file        segment.h
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

#ifndef NETWORK_SEGMENT_H
#define NETWORK_SEGMENT_H

#include <common.h>

#include <core/objects/item_buffer.h>
#include <core/objects/brick.h>

class Segment
{
public:
    ItemBuffer bricks;

    Brick** nodeBricks = nullptr;
    uint32_t numberOfNodeBricks = 0;
    Brick** inputBricks = nullptr;
    uint32_t numberOfInputBricks = 0;
    Brick** outputBricks = nullptr;
    uint32_t numberOfOutputBricks = 0;

    // host-representation of permanent gpu-data
    ItemBuffer nodes;
    ItemBuffer synapses;

    ItemBuffer nodeProcessingBuffer;
    ItemBuffer nodeInputBuffer;
    ItemBuffer nodeOutputBuffer;

    std::atomic_flag input_lock = ATOMIC_FLAG_INIT;

    // other
    ItemBuffer randomIntValues;
    ItemBuffer globalValues;

    Segment();

    DataItem* getMetadata();

    bool connectBricks(const uint32_t sourceBrickId,
                       const uint8_t sourceSide,
                       const uint32_t targetBrickId);
    bool disconnectBricks(const uint32_t sourceBrickId,
                          const uint8_t sourceSide);

};

#endif // NETWORK_SEGMENT_H
