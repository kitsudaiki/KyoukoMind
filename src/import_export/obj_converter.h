/**
 * @file        obj_converter.h
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

#ifndef OBJ_CONVERTER_H
#define OBJ_CONVERTER_H

#include <common.h>
#include <libKitsunemimiObj/obj_item.h>

using Kitsunemimi::Obj::Vec4;
using Kitsunemimi::Obj::ObjItem;

struct Brick;
struct Node;
struct EdgeSection;

// convert whole network
void convertNetworkToString(std::string &result);
void convertNetworkToObj(ObjItem &result);

// convert one brick
void convertBrickToString(std::string &result,
                          const uint32_t brickId);
void convertBrickToObj(ObjItem &result,
                       const uint32_t brickId);
void convertBrickToObj(ObjItem &result,
                       Brick* brick);

// convert one node
void convertNodeToString(std::string &result,
                         const uint32_t brickId,
                         const uint32_t nodeId);
void convertNodeToObj(ObjItem &result,
                      const uint32_t brickId,
                      const uint32_t nodeId);
void convertNodeToObj(ObjItem &result,
                      Brick* brick,
                      const uint32_t nodeId);

// convert one edge
void convertEdgesToObj(ObjItem &result,
                       EdgeSection* section,
                       const uint16_t pos,
                       const uint32_t vectorPos);

#endif // OBJ_CONVERTER_H
