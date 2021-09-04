/**
 * @file        abstract_segment.cpp
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

#include "abstract_segment.h"

#include <core/objects/network_cluster.h>

/**
 * @brief constructor
 */
AbstractSegment::AbstractSegment() {}

/**
 * @brief destructor
 */
AbstractSegment::~AbstractSegment() {}

/**
 * @brief AbstractSegment::getType
 * @return
 */
SegmentTypes
AbstractSegment::getType() const
{
    return m_type;
}

/**
 * @brief AbstractSegment::isReady
 * @return
 */
bool
AbstractSegment::isReady()
{
    for(uint8_t i = 0; i < 12; i++)
    {
        if(segmentNeighbors->neighbors[i].inUse == 1
                && segmentNeighbors->neighbors[i].inputReady == 0)
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief AbstractSegment::finishSegment
 * @return
 */
bool
AbstractSegment::finishSegment()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    float* sourceBuffer = nullptr;
    float* targetBuffer  = nullptr;
    uint32_t targetId = 0;
    uint8_t targetSide = 0;
    AbstractSegment* targetSegment = nullptr;
    SegmentNeighborList* targetNeighbors = nullptr;

    for(uint8_t i = 0; i < 12; i++)
    {
        if(segmentNeighbors->neighbors[i].inUse == 1)
        {
            // get information of the neighbor
            sourceBuffer = segmentNeighbors->neighbors[i].outputTransferBuffer;
            targetId = segmentNeighbors->neighbors[i].targetSegmentId;
            targetSide = segmentNeighbors->neighbors[i].targetSide;

            // copy data to the target buffer and wipe the source buffer
            targetSegment = cluster->allSegments[targetId];
            targetNeighbors = targetSegment->segmentNeighbors;
            targetBuffer = targetNeighbors->neighbors[targetSide].inputTransferBuffer;
            memcpy(targetBuffer,
                   sourceBuffer,
                   segmentNeighbors->neighbors[i].size * sizeof(float));
            memset(sourceBuffer,
                   0,
                   segmentNeighbors->neighbors[i].size * sizeof(float));

            // mark the target as ready for processing
            targetSegment->segmentNeighbors->neighbors[targetSide].inputReady = true;
        }
    }

    return true;
}

/**
 * @brief AbstractSegment::initPosition
 * @param parsedContent
 * @return
 */
bool
AbstractSegment::initPosition(const JsonItem &parsedContent)
{
    JsonItem paredPosition = parsedContent.get("position");
    segmentHeader->position.x = paredPosition.get(0).getInt();
    segmentHeader->position.y = paredPosition.get(1).getInt();
    segmentHeader->position.z = paredPosition.get(2).getInt();

    return true;
}

/**
 * @brief AbstractSegment::initBorderBuffer
 * @param parsedContent
 * @return
 */
bool
AbstractSegment::initBorderBuffer(const JsonItem &parsedContent)
{
    uint64_t posCounter = 0;
    JsonItem neighbors = parsedContent.get("neighbors");

    for(uint32_t i = 0; i < 12; i++)
    {
        JsonItem currentDef = neighbors.get(i);
        const uint32_t next = currentDef.get("id").getLong();
        const std::string direction = currentDef.get("direction").getString();
        const uint32_t size  = currentDef.get("size").getLong();

        if(next != UNINIT_STATE_32)
        {
            SegmentNeighbor* currentNeighbor = &segmentNeighbors->neighbors[i];
            currentNeighbor->inUse = true;
            currentNeighbor->size = size;
            currentNeighbor->targetSegmentId = next;
            currentNeighbor->targetSide = 11 - i;
            currentNeighbor->inputTransferBuffer = &inputTransfers[posCounter];
            currentNeighbor->outputTransferBuffer = &outputTransfers[posCounter];

            if(direction == "input") {
                currentNeighbor->direction = INPUT_DIRECTION;
            }
            if(direction == "output") {
                currentNeighbor->direction = OUTPUT_DIRECTION;
            }

            posCounter += size;
        }
    }

    return true;
}

/**
 * @brief AbstractSegment::createGenericNewHeader
 * @param header
 * @param borderbufferSize
 * @return
 */
uint32_t
AbstractSegment::createGenericNewHeader(SegmentHeader &header,
                                        const uint64_t borderbufferSize)
{
    uint32_t segmentDataPos = 0;

    // init header
    segmentDataPos += 1 * sizeof(SegmentHeader);

    // init settings
    header.settings.count = 1;
    header.settings.bytePos = segmentDataPos;
    segmentDataPos += 1 * sizeof(SegmentSettings);

    // init neighborList
    header.neighborList.count = 1;
    header.neighborList.bytePos = segmentDataPos;
    segmentDataPos += 1 * sizeof(SegmentNeighborList);

    // init inputTransfers
    header.inputTransfers.count = borderbufferSize;
    header.inputTransfers.bytePos = segmentDataPos;
    segmentDataPos += borderbufferSize * sizeof(float);

    // init outputTransfers
    header.outputTransfers.count = borderbufferSize;
    header.outputTransfers.bytePos = segmentDataPos;
    segmentDataPos += borderbufferSize * sizeof(float);

    return segmentDataPos;
}
