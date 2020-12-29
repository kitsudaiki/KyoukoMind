/**
 * @file        segment_initializing.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2018 Tobias Anker
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

#ifndef SEGMENT_INITIALIZING_H
#define SEGMENT_INITIALIZING_H

#include <common.h>

class Segment;

bool initBricks(Segment &segment,
                const uint32_t numberOfBricks);
bool initGlobalValues(Segment &segment);
bool initNodeBlocks(Segment &segment,
                    const uint32_t &numberOfNodes);
bool initRandomValues(Segment &segment);
bool initEdgeSectionBlocks(Segment &segment,
                           const uint32_t numberOfEdgeSections);
bool initSynapseSectionBlocks(Segment &segment,
                              const uint32_t numberOfSynapseSections);
bool initTransferBlocks(Segment &segment,
                        const uint32_t totalNumberOfAxons,
                        const uint64_t maxNumberOySynapseSections);

#endif // SEGMENT_INITIALIZING_H
