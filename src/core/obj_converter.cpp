#include "obj_converter.h"

#include <root_object.h>
#include <core/brick_handler.h>

#include <core/objects/brick.h>
#include <core/objects/brick_pos.h>
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
    if(brick->dataConnections[NODE_DATA].inUse == 0) {
        return;
    }

    for(uint16_t i = 0; i < NUMBER_OF_NODES_PER_BRICK; i++)
    {
        DataConnection* data = &brick->dataConnections[NODE_DATA];

        Node* start = static_cast<Node*>(data->buffer.data);
        Node* end = start + data->numberOfItems;

        // iterate over all nodes in the brick
        for(Node* node = start;
            node < end;
            node++)
        {
            convertNodeToObj(result, brick, node);
        }
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

    if(brick->dataConnections[NODE_DATA].inUse == 0
            || nodeId > NUMBER_OF_NODES_PER_BRICK)
    {
        return;
    }

    DataConnection* data = &brick->dataConnections[NODE_DATA];
    Node* nodeArray = static_cast<Node*>(data->buffer.data);
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

    uint64_t targetBrickPath = node->targetBrickPath;
    while(targetBrickPath != 0)
    {
        const uint8_t side = targetBrickPath % 32;
        Brick* nextBrick = brick->neighbors[side].targetBrick;

        const Vec4 nextVec = convertPos(nextBrick->brickPos);
        result.vertizes.push_back(nextVec);

        std::vector<uint32_t> linePart;
        const uint32_t numberOfVertizes = static_cast<uint32_t>(result.vertizes.size());
        linePart.push_back(numberOfVertizes - 2);
        linePart.push_back(numberOfVertizes - 1);
        result.lines.push_back(linePart);

        targetBrickPath = targetBrickPath >> 5;
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

    vec.x = static_cast<float>(pos.x) + (1.0f / static_cast<float>(rand()));
    vec.y = static_cast<float>(pos.y) + (1.0f / static_cast<float>(rand()));
    vec.z = 1.0f / static_cast<float>(rand());

    return vec;
}

}
