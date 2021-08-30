/**
 * @file        dynamic_segment.h
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

#ifndef DYNAMIC_SEGMENTS_H
#define DYNAMIC_SEGMENTS_H

#include <common.h>

#include <core/objects/segments/abstract_segment.h>

class DynamicSegment : public AbstractSegment
{
public:
    DynamicSegment();
    ~DynamicSegment();

    bool initSegment(JsonItem &parsedContent);
    bool connectBorderBuffer();

    Brick* bricks = nullptr;
    uint32_t* brickOrder = nullptr;
    Node* nodes = nullptr;

    SynapseSection* synapseSections = nullptr;

private:
    SegmentSettings initSettings(JsonItem &parsedContent);
    SegmentHeader createNewHeader(const uint32_t numberOfBricks,
                                  const uint32_t numberOfNodes,
                                  const uint64_t numberOfSynapseSections,
                                  const uint64_t borderbufferSize);
    void initSegmentPointer(const SegmentHeader &header);
    void allocateSegment(SegmentHeader &header);
    void initDefaultValues(const uint32_t numberOfBricks,
                           const uint32_t numberOfNodes);

    void addBricksToSegment(const JsonItem &metaBase);
    bool initTargetBrickList();

    Brick createNewBrick(const JsonItem &brickDef, const uint32_t id);
    void connectBrick(Brick *sourceBrick, const uint8_t side);
    void connectAllBricks();
    Position getNeighborPos(Position sourcePos, const uint8_t side);
};

#endif // DYNAMIC_SEGMENTS_H
