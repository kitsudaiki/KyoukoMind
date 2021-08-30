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

AbstractSegment::AbstractSegment()
{

}

AbstractSegment::~AbstractSegment()
{

}

SegmentTypes
AbstractSegment::getType() const
{
    return m_type;
}

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

bool
AbstractSegment::initPosition(JsonItem &parsedContent)
{
    JsonItem paredPosition = parsedContent.get("position");
    segmentHeader->position.x = paredPosition[0].getInt();
    segmentHeader->position.y = paredPosition[1].getInt();
    segmentHeader->position.z = paredPosition[2].getInt();

    return true;
}

bool
AbstractSegment::initBorderBuffer(JsonItem &parsedContent)
{
    uint64_t posCounter = 0;

    JsonItem neighbors = parsedContent.get("neighbors");

    for(uint32_t i = 0; i < 12; i++)
    {
        JsonItem currentNeighbor = neighbors.get(i);

        const uint32_t next = currentNeighbor.get("id").getLong();
        if(next != UNINIT_STATE_32)
        {
            const uint32_t size  = currentNeighbor.get("size").getLong();
            segmentNeighbors->neighbors[i].inUse = true;
            segmentNeighbors->neighbors[i].size = size;
            segmentNeighbors->neighbors[i].targetSide = 11 - i;
            segmentNeighbors->neighbors[i].inputTransferBuffer = &transferEntries[posCounter];
            posCounter += size;
            segmentNeighbors->neighbors[i].outputTransferBuffer = &transferEntries[posCounter];
            posCounter += size;
        }
    }

    return true;
}
