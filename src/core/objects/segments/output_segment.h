/**
 * @file        output_segment.h
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

#ifndef OUTPUT_SEGMENTS_H
#define OUTPUT_SEGMENTS_H

#include <common.h>

#include <core/objects/segments/abstract_segment.h>

class OutputSegment : public AbstractSegment
{
public:
    OutputSegment();
    ~OutputSegment();

    float lastTotalError = 0.0f;
    float actualTotalError = 0.0f;

    OutputNode* outputs = nullptr;

    bool initSegment(JsonItem &parsedContent);
    bool connectBorderBuffer();

private:
    SegmentHeader createNewHeader(const uint32_t numberOfOutputs,
                                  const uint64_t borderbufferSize);
    void initSegmentPointer(const SegmentHeader &header);
    void allocateSegment(SegmentHeader &header);
};

#endif // OUTPUT_SEGMENTS_H