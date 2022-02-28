/**
 * @file        static_segment.h
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

#ifndef KYOUKOMIND_STATIC_SEGMENTS_H
#define KYOUKOMIND_STATIC_SEGMENTS_H

#include <common.h>

#include <core/segments/abstract_segment.h>

#include "node.h"

class StaticSegment
        : public AbstractSegment
{
public:
    StaticSegment();
    ~StaticSegment();

    bool initSegment(const JsonItem &parsedContent);

    Brick* bricks = nullptr;
    StaticNode* nodes = nullptr;
    float* connections = nullptr;

private:
    StaticSegmentSettings initSettings(const JsonItem &);
    SegmentHeader createNewHeader(const uint32_t numberOfBricks,
                                  const uint32_t numberOfConnections,
                                  const uint32_t numberOfNodes,
                                  const uint64_t borderbufferSize);
    void initSegmentPointer(const SegmentHeader &header);
    bool connectBorderBuffer();
    void allocateSegment(SegmentHeader &header);
    void initDefaultValues();

    void addBricksToSegment(const JsonItem &metaBase);
    Brick createNewBrick(const JsonItem &brickDef, const uint32_t id);
    bool initializeNodes();
};

#endif // KYOUKOMIND_STATIC_SEGMENTS_H
