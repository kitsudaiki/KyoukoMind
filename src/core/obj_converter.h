/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef OBJ_CONVERTER_H
#define OBJ_CONVERTER_H

#include <common.h>
#include <libKitsunemimiObj/obj_item.h>

using Kitsunemimi::Obj::Vec4;
using Kitsunemimi::Obj::ObjItem;

namespace KyoukoMind
{
class Brick;
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

}

#endif // OBJ_CONVERTER_H
