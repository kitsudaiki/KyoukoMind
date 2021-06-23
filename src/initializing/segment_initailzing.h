/**
 * @file        segment_initailzing.h
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

#ifndef SEGMENT_INITAILZING_H
#define SEGMENT_INITAILZING_H

#include <common.h>

#include <core/objects/segment.h>

namespace Kitsunemimi {
namespace Ai {
struct AiBaseMeta;
}
}

Segment* initSynapseSegment(const uint32_t numberOfNodeBricks,
                                const uint32_t numberOfNodes,
                                const uint64_t numberOfSynapseSections,
                                const uint32_t numberOfInputs,
                                const uint32_t numberOfOutputs,
                                const uint32_t numberOfRandValues);

bool initializeNodes(Segment &segment,
                     Kitsunemimi::Ai::InitSettings* initMetaData);

void addBricksToSegment(Segment &segment,
                        Kitsunemimi::Ai::InitSettings *initMetaData,
                        const Kitsunemimi::Ai::AiBaseMeta& metaBase);

#endif // SEGMENT_INITAILZING_H
