/**
 * @file        cluster_parsing.h
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

#ifndef KYOUKOMIND_CLUSTER_PARSING_H
#define KYOUKOMIND_CLUSTER_PARSING_H

#include <common.h>

const std::string prepareDirection(const JsonItem &currentSegment,
                                   const JsonItem &segments,
                                   const uint32_t foundNext,
                                   const uint8_t side);
long getNeighborBorderSize(const JsonItem &currentSegment,
                           const JsonItem &segments,
                           const uint32_t foundNext);
bool prepareSingleSegment(std::deque<uint32_t> &segmentQueue,
                          const JsonItem &segments,
                          JsonItem &parsedSegments);
bool prepareSegments(const JsonItem &parsedContent);
uint32_t checkNextPosition(const JsonItem &segments, const Position nextPos);
Position convertPosition(const JsonItem &parsedContent);


#endif // KYOUKOMIND_CLUSTER_PARSING_H
