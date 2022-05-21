/**
 * @file        segment_meta.h
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

#ifndef KYOUKOMIND_SEGMENT_META_H
#define KYOUKOMIND_SEGMENT_META_H

#include <common.h>

enum SegmentTypes
{
    UNDEFINED_SEGMENT = 0,
    INPUT_SEGMENT = 1,
    OUTPUT_SEGMENT = 2,
    DYNAMIC_SEGMENT = 3,
};

struct SegmentHeaderEntry
{
    uint64_t bytePos = 0;
    uint64_t count = 0;

    // total size: 16 Byte
};

struct SegmentHeader
{
    uint8_t objectType = SEGMENT_OBJECT;
    uint8_t version = 1;
    uint8_t segmentType = UNDEFINED_SEGMENT;
    uint8_t padding;
    uint32_t segmentID = UNINIT_STATE_32;
    uint64_t staticDataSize = 0;
    Position position;

    Kitsunemimi::Hanami::kuuid parentClusterId;

    // synapse-segment
    SegmentHeaderEntry settings;
    SegmentHeaderEntry neighborList;
    SegmentHeaderEntry inputTransfers;
    SegmentHeaderEntry outputTransfers;

    SegmentHeaderEntry bricks;
    SegmentHeaderEntry brickOrder;
    SegmentHeaderEntry nodes;
    SegmentHeaderEntry inputs;
    SegmentHeaderEntry outputs;

    SegmentHeaderEntry synapseSections;
    SegmentHeaderEntry connections;

    uint8_t padding2[8];

    // total size: 256 Byte
};

enum NeighborDirection
{
    UNDEFINED_DIRECTION = 0,
    INPUT_DIRECTION = 1,
    OUTPUT_DIRECTION = 2,
};

struct SegmentNeighbor
{
    uint32_t targetSegmentId = UNINIT_STATE_32;

    uint8_t targetSide = 0;
    uint8_t direction = UNDEFINED_DIRECTION;

    bool inUse = false;
    bool inputReady = false;

    uint32_t size = 0;
    uint8_t padding[4];

    float* inputTransferBuffer = nullptr;
    float* outputTransferBuffer = nullptr;

    // total size: 32 Byte
};

struct SegmentNeighborList
{
    SegmentNeighbor neighbors[12];

    uint8_t padding[128];

    // total size: 512 Byte
};

#endif // SEGMENT_META_H
