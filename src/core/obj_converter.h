#ifndef OBJ_CONVERTER_H
#define OBJ_CONVERTER_H

#include <common.h>
#include <libKitsunemimiObj/obj_item.h>
#include <core/objects/brick_pos.h>

using Kitsunemimi::Obj::Vec4;
using Kitsunemimi::Obj::ObjItem;

namespace KyoukoMind
{
struct Brick;
struct Node;

// convert whole network
void convertNetworkToString(std::string &result);
void convertNetworkToObj(ObjItem &result);

// convert one brick
void convertBrickToString(std::string &result,
                          const BrickID brickId);
void convertBrickToObj(ObjItem &result,
                       const BrickID brickId);
void convertBrickToObj(ObjItem &result,
                       Brick* brick);

// convert one node
void convertNodeToString(std::string &result,
                         const BrickID brickId,
                         const uint16_t nodeId);
void convertNodeToObj(ObjItem &result,
                      const BrickID brickId,
                      const uint16_t nodeId);
void convertNodeToObj(ObjItem &result,
                      Brick* brick,
                      Node* node);

Brick* convertAxonToObj(ObjItem &result,
                        Brick* brick,
                        uint64_t path);

void convertEdgesToObj(ObjItem &result,
                       Brick* brick,
                       const uint32_t id,
                       const uint32_t vecPos);

Kitsunemimi::Obj::Vec4 convertPos(const BrickPos pos);

}

#endif // OBJ_CONVERTER_H
