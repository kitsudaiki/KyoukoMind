/**
 * @file        abstract_segment.h
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

#ifndef KYOUKOMIND_ABSTRACT_SEGMENTS_H
#define KYOUKOMIND_ABSTRACT_SEGMENTS_H

#include <common.h>

#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiCommon/buffer/item_buffer.h>

#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>

//==================================================================================================

enum SegmentTypes
{
    UNDEFINED_SEGMENT = 0,
    INPUT_SEGMENT = 1,
    OUTPUT_SEGMENT = 2,
    DYNAMIC_SEGMENT = 3,
};

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
    uint64_t maxSynapseSections = 0;
    uint8_t refractionTime = 1;
    uint8_t multiplicatorRange = 0;
    uint8_t doLearn = 0;

    uint8_t padding[213];

    // total size: 256 Byte
};

struct SegmentHeaderEntry
{
    uint64_t bytePos = 0;
    uint64_t count = 0;

    // total size: 16 Byte
};

struct SegmentHeader
{
    uint32_t segmentID = UNINIT_STATE_32;
    uint8_t segmentType = UNDEFINED_SEGMENT;
    uint8_t padding1[3];
    uint64_t staticDataSize = 0;
    Position position;

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

    uint8_t padding2[64];

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

//==================================================================================================

class AbstractSegment
{
public:
    AbstractSegment();
    virtual ~AbstractSegment();

    SegmentTypes getType() const;

    Kitsunemimi::ItemBuffer segmentData;
    Kitsunemimi::DataBuffer staticSegmentData;

    SegmentHeader* segmentHeader = nullptr;
    SegmentSettings* segmentSettings = nullptr;
    SegmentNeighborList* segmentNeighbors = nullptr;
    float* inputTransfers = nullptr;
    float* outputTransfers = nullptr;

    virtual bool initSegment(JsonItem &parsedContent) = 0;
    virtual bool connectBorderBuffer() = 0;

    bool isReady();
    bool finishSegment();

protected:
    SegmentTypes m_type = UNDEFINED_SEGMENT;

    bool initPosition(JsonItem &parsedContent);
    bool initBorderBuffer(JsonItem &parsedContent);

    uint32_t createGenericNewHeader(SegmentHeader &header,
                                    const uint64_t borderbufferSize);
};

//==================================================================================================

#endif // KYOUKOMIND_ABSTRACT_SEGMENTS_H
