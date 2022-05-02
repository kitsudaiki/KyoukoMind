/**
 * @file        template_creator.h
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

#ifndef TEMPLATECREATOR_H
#define TEMPLATECREATOR_H

#include <common.h>

Kitsunemimi::DataMap* generateNewTemplate(const std::string name,
                                          const long numberOfInputNodes,
                                          const long numberOfOutputNodes,
                                          const Kitsunemimi::Json::JsonItem &settingsOverride);
void createClusterSettings(Kitsunemimi::DataMap* result);
void createSegments(Kitsunemimi::DataMap* result,
                    const long numberOfInputNodes,
                    const long numberOfOutputNodes,
                    const Kitsunemimi::Json::JsonItem &settingsOverride);
uint32_t createInputSegments(Kitsunemimi::DataArray* result,
                             const long numberOfInputNodes);

void createDynamicSegments(Kitsunemimi::DataArray* result,
                           const long numberOfInputNodes,
                           const long numberOfOutputNodes,
                           const Kitsunemimi::Json::JsonItem &settingsOverride);
void createSegmentSettings(Kitsunemimi::DataMap* result,
                           const Kitsunemimi::Json::JsonItem &settingsOverride);
void createSegmentBricks(Kitsunemimi::DataMap* result,
                         const long numberOfInputNodes,
                         const long numberOfOutputNodes);
void createSegmentBricksOld(Kitsunemimi::DataMap* result,
                         const long numberOfInputNodes,
                         const long numberOfOutputNodes);

uint32_t createOutputSegments(Kitsunemimi::DataArray* result,
                              const long numberOfOutputNodes);

Kitsunemimi::DataArray* createPosition(const long x,
                                       const long y,
                                       const long z);
#endif // TEMPLATECREATOR_H