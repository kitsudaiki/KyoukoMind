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
#include <core/objects/node.h>
#include <core/objects/synapses.h>

//==================================================================================================

struct SegmentSettings
{
    float synapseDeleteBorder = 1.0f;
    float actionPotential = 100.0f;
    float nodeCooldown = 100.0f;
    float memorizing = 0.1f;
    float gliaValue = 1.0f;
    float signNeg = 0.6f;
    float potentialOverflow = 20.0f;
    float maxSynapseWeight = 30.0f;
    uint8_t refractionTime = 1;
    uint8_t multiplicatorRange = 0;
    uint8_t doLearn = 0;

    uint8_t padding[221];
};

struct SegmentHeaderEntry
{
    uint64_t bytePos = 0;
    uint64_t count = 0;

    // total size: 16 Byte
};

struct SegmentHeader
{
    uint64_t segmentDataSize = 0;

    // synapse-segment
    SegmentHeaderEntry settings;
    SegmentHeaderEntry bricks;
    SegmentHeaderEntry brickOrder;
    SegmentHeaderEntry synapseSections;
    SegmentHeaderEntry nodes;
    SegmentHeaderEntry inputs;
    SegmentHeaderEntry outputs;

    uint8_t padding[136];

    // total size: 256 Byte
};

struct Segment
{
    Kitsunemimi::DataBuffer persistenBuffer;
    Kitsunemimi::DataBuffer ephemeralBuffer;

    SegmentHeader* segmentHeader = nullptr;
    SegmentSettings* synapseSettings = nullptr;
    Brick* bricks = nullptr;
    uint32_t* brickOrder = nullptr;
    Node* nodes = nullptr;
    SynapseSection* synapseSections = nullptr;
    InputNode* inputs = nullptr;
    OutputNode* outputs = nullptr;

    Segment() {}
};

//==================================================================================================

#endif // NETWORK_SEGMENT_H
