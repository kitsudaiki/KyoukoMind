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

#include <libKitsunemimiCommon/buffer/item_buffer.h>
#include <core/objects/brick.h>

class Segment
{
public:
    Kitsunemimi::ItemBuffer bricks;

    Brick** nodeBricks = nullptr;
    uint32_t numberOfNodeBricks = 0;
    Brick** inputBricks = nullptr;
    uint32_t numberOfInputBricks = 0;
    Brick** outputBricks = nullptr;
    uint32_t numberOfOutputBricks = 0;

    // host-representation of permanent gpu-data
    Kitsunemimi::ItemBuffer nodes;
    Kitsunemimi::ItemBuffer synapses;
    Kitsunemimi::ItemBuffer outputSynapses;
    Kitsunemimi::ItemBuffer outputs;

    Kitsunemimi::ItemBuffer nodeProcessingBuffer;
    Kitsunemimi::ItemBuffer nodeInputBuffer;
    Kitsunemimi::ItemBuffer nodeOutputBuffer;


    uint32_t nodesPerBrick = 0;

    std::atomic_flag input_lock = ATOMIC_FLAG_INIT;

    // other
    Kitsunemimi::ItemBuffer randomIntValues;
    Kitsunemimi::ItemBuffer globalValues;

    Segment();

    bool initializeBuffer(const uint32_t numberOfBricks,
                          const uint32_t numberOfNodeBricks,
                          const uint32_t numberOfNodes,
                          const uint32_t numberOfSynapseSections,
                          const uint32_t numberOfOutputBricks,
                          const uint32_t numberOfOutputs,
                          const uint32_t numberOfRandValues);

    DataItem* getMetadata();

    bool connectBricks(const uint32_t sourceBrickId,
                       const uint8_t sourceSide,
                       const uint32_t targetBrickId);
    bool disconnectBricks(const uint32_t sourceBrickId,
                          const uint8_t sourceSide);

private:
    bool initNodeBuffer(Kitsunemimi::ItemBuffer &nodeBuffer,
                        const uint32_t numberOfItems);
};

#endif // NETWORK_SEGMENT_H
