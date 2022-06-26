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

#include <core/segments/brick.h>
#include <core/segments/segment_meta.h>

#include <core/segments/dynamic_segment/objects.h>

class Cluster;

class AbstractSegment
{
public:
    AbstractSegment();
    AbstractSegment(const void* data, const uint64_t dataSize);
    virtual ~AbstractSegment();

    SegmentTypes getType() const;

    Kitsunemimi::ItemBuffer segmentData;

    SegmentHeader* segmentHeader = nullptr;
    DynamicSegmentSettings* dynamicSegmentSettings = nullptr;

    SegmentNeighborList* segmentNeighbors = nullptr;
    float* inputTransfers = nullptr;
    float* outputTransfers = nullptr;
    Cluster* parentCluster = nullptr;

    virtual bool initSegment(const JsonItem &parsedContent) = 0;
    virtual bool reinitPointer(const uint64_t numberOfBytes) = 0;

    bool isReady();
    void finishSegment();

protected:
    SegmentTypes m_type = UNDEFINED_SEGMENT;

    uint32_t createGenericNewHeader(SegmentHeader &header,
                                    const uint64_t borderbufferSize);
    Position convertPosition(const JsonItem &parsedContent);
    bool initBorderBuffer(const JsonItem &parsedContent);
    bool reinitGenericPointer();

private:
    virtual void initSegmentPointer(const SegmentHeader &header) = 0;
    virtual bool connectBorderBuffer() = 0;
    virtual void allocateSegment(SegmentHeader &header) = 0;
};

//==================================================================================================

#endif // KYOUKOMIND_ABSTRACT_SEGMENTS_H
