#include "obj_converter.h"

#include <root_object.h>
#include <core/brick_handler.h>

#include <core/objects/brick.h>
#include <core/objects/brick_pos.h>
#include <core/objects/edges.h>
#include <core/objects/data_connection.h>

namespace KyoukoMind
{

/**
 * @brief convertNetworkToObj
 * @param result
 */
void
convertNetworkToString(std::string &result)
{
    ObjItem item;
    convertNetworkToObj(item);
    Kitsunemimi::Obj::convertToString(result, item);
}

/**
 * @brief convertNetworkToObj
 * @param result
 */
void
convertNetworkToObj(ObjItem &result)
{
    BrickHandler* handler = RootObject::m_brickHandler;
    const uint64_t numberOfBricks = handler->getNumberOfBrick();
    for(uint64_t i = 0; i < numberOfBricks; i++)
    {
        Brick* brick = handler->getBrickByIndex(i);
        convertBrickToObj(result, brick);
    }
}

/**
 * @brief convertBrickToString
 * @param result
 * @param brickId
 */
void
convertBrickToString(std::string &result,
                     const BrickID brickId)
{
    ObjItem item;
    convertBrickToObj(item, brickId);
    Kitsunemimi::Obj::convertToString(result, item);
}

/**
 * @brief convertBrickToObj
 * @param result
 * @param brickId
 */
void
convertBrickToObj(ObjItem &result,
                  const BrickID brickId)
{
    convertBrickToObj(result, RootObject::m_brickHandler->getBrick(brickId));
}

/**
 * @brief convertBrickToObj
 * @param result
 * @param brick
 */
void
convertBrickToObj(ObjItem &result,
                  Brick* brick)
{
    if(brick->nodes == nullptr) {
        return;
    }

    Node* start = brick->nodes;
    Node* end = start + NUMBER_OF_NODES_PER_BRICK;

    // iterate over all nodes in the brick
    for(Node* node = start;
        node < end;
        node++)
    {
        convertNodeToObj(result, brick, node);
    }
}

/**
 * @brief convertNodeToString
 * @param result
 * @param brickId
 * @param nodeId
 */
void
convertNodeToString(std::string &result,
                    const BrickID brickId,
                    const uint16_t nodeId)
{
    ObjItem item;
    convertNodeToObj(item, brickId, nodeId);
    Kitsunemimi::Obj::convertToString(result, item);
}

/**
 * @brief convertNodeToObj
 * @param result
 * @param brickId
 * @param nodeId
 */
void
convertNodeToObj(ObjItem &result,
                 const BrickID brickId,
                 const uint16_t nodeId)
{
    Brick* brick = RootObject::m_brickHandler->getBrick(brickId);

    if(brick->nodes == nullptr
            || nodeId > NUMBER_OF_NODES_PER_BRICK)
    {
        return;
    }

    Node* nodeArray = brick->nodes;
    Node* node = &nodeArray[nodeId];

    convertNodeToObj(result, brick, node);
}

/**
 * @brief convertNodeToObj
 * @param result
 * @param brick
 * @param nodeId
 */
void convertNodeToObj(ObjItem &result,
                      Brick* brick,
                      Node* node)
{
    const Vec4 vec = convertPos(brick->brickPos);
    result.vertizes.push_back(vec);

    uint64_t targetBrickPath = node->targetBrickPath / 32;
    Brick* axonEnd = convertAxonToObj(result, brick, targetBrickPath);

    result.vertizes.push_back(convertPos(axonEnd->brickPos));
    const uint32_t vecPos = static_cast<uint32_t>(result.vertizes.size());

    convertEdgesToObj(result,
                      axonEnd,
                      node->targetAxonId,
                      vecPos);
}

/**
 * @brief convertAxonToObj
 * @param result
 * @param brick
 * @param path
 */
Brick*
convertAxonToObj(ObjItem &result,
                 Brick* brick,
                 uint64_t path)
{
    Brick* currentBrick = brick;
    while(path != 0)
    {
        const uint8_t side = path & 0x1F;
        Brick* nextBrick = currentBrick->neighbors[side].targetBrick;
        assert(nextBrick != nullptr);

        const Vec4 nextVec = convertPos(nextBrick->brickPos);
        result.vertizes.push_back(nextVec);

        std::vector<uint32_t> linePart;
        const uint32_t numberOfVertizes = static_cast<uint32_t>(result.vertizes.size());
        linePart.push_back(numberOfVertizes - 1);
        linePart.push_back(numberOfVertizes);
        result.lines.push_back(linePart);

        currentBrick = nextBrick;
        path = path >> 5;
    }

    return currentBrick;
}

/**
 * @brief convertEdgesToObj
 * @param result
 * @param brick
 * @param node
 */
void
convertEdgesToObj(ObjItem &result,
                  Brick *brick,
                  const uint32_t id,
                  const uint32_t vecPos)
{
    EdgeSection* section = &getEdgeBlock(brick->edges)[id];

    for(uint8_t side = 2; side < 23; side++)
    {
        const Edge tempEdge = section->edges[side];
        if(tempEdge.available == 0) {
            continue;
        }

        if(side == 22)
        {
            if(tempEdge.targetId == UNINIT_STATE_32){
                continue;
            }

            const Vec4 sameVec = convertPos(brick->brickPos);
            result.vertizes.push_back(sameVec);

            std::vector<uint32_t> linePart;
            const uint32_t numberOfVertizes = static_cast<uint32_t>(result.vertizes.size());
            linePart.push_back(vecPos);
            linePart.push_back(numberOfVertizes);
            result.lines.push_back(linePart);

        }
        else
        {
            if(tempEdge.targetId != UNINIT_STATE_32)
            {
                Brick* nextBrick = brick->neighbors[side].targetBrick;

                result.vertizes.push_back(convertPos(nextBrick->brickPos));
                const uint32_t nextVecPos = static_cast<uint32_t>(result.vertizes.size());

                convertEdgesToObj(result,
                                  nextBrick,
                                  tempEdge.targetId,
                                  nextVecPos);

                std::vector<uint32_t> linePart;
                linePart.push_back(vecPos);
                linePart.push_back(nextVecPos);
                result.lines.push_back(linePart);
            }
        }
    }
}

/**
 * @brief convertPos
 * @param pos
 * @return
 */
Vec4
convertPos(const BrickPos pos)
{
    Kitsunemimi::Obj::Vec4 vec;

    vec.x = static_cast<float>(pos.x)
            + static_cast <float>(rand()) / static_cast <float> (0x7FFFFFFF);
    vec.y = static_cast<float>(pos.y)
            + static_cast <float>(rand()) / static_cast <float> (0x7FFFFFFF);
    vec.z = static_cast <float>(rand()) / static_cast <float> (0x7FFFFFFF);

    return vec;
}

}
