/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "obj_converter.h"

#include <kyouko_root.h>

#include <core/processing/objects/edges.h>
#include <core/processing/objects/node.h>
#include <core/processing/cpu/edge_processing.h>

#include <core/object_handling/brick.h>
#include <core/object_handling/item_buffer.h>
#include <core/object_handling/segment.h>
#include <core/object_handling/global_values.h>

namespace KyoukoMind
{

/**
 * @brief convertPos
 * @param pos
 * @return
 */
Vec4
convertPos(const Brick::BrickPos pos)
{
    Kitsunemimi::Obj::Vec4 vec;

    vec.x = static_cast<float>(pos.x)
            + static_cast <float>(rand()) / static_cast <float> (0x7FFFFFFF);
    vec.y = static_cast<float>(pos.y)
            + static_cast <float>(rand()) / static_cast <float> (0x7FFFFFFF);
    vec.z = static_cast <float>(rand()) / static_cast <float> (0x7FFFFFFF);

    return vec;
}

//--------------------------------------------------------------------------------------------------

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
    const uint64_t numberOfBricks = KyoukoRoot::m_segment->bricks.itemCapacity;

    for(uint64_t i = 0; i < numberOfBricks; i++)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[i];
        convertBrickToObj(result, brick);
    }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief convertBrickToString
 * @param result
 * @param brickId
 */
void
convertBrickToString(std::string &result,
                     const uint32_t brickId)
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
                  const uint32_t brickId)
{
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[brickId];
    convertBrickToObj(result, brick);
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
    if(brick->nodeBrickId == UNINIT_STATE_32) {
        return;
    }

    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    for(uint32_t i = 0; i < globalValues->numberOfNodesPerBrick; i++)
    {
        AxonTransfer* axons = getBuffer<AxonTransfer>(KyoukoRoot::m_segment->axonTransfers);
        if(axons[brick->nodeBrickId * globalValues->numberOfNodesPerBrick + i].weight <= 0.0f) {
            continue;
        }

        convertNodeToObj(result, brick, brick->brickId * globalValues->numberOfNodesPerBrick + i);
    }
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief convertNodeToString
 * @param result
 * @param brickId
 * @param nodeId
 */
void
convertNodeToString(std::string &result,
                    const uint32_t brickId,
                    const uint32_t nodeId)
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
                 const uint32_t brickId,
                 const uint32_t nodeId)
{
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[brickId];
    convertNodeToObj(result, brick, nodeId);
}

/**
 * @brief convertNodeToObj
 * @param result
 * @param brick
 * @param nodeId
 */
void
convertNodeToObj(ObjItem &result,
                 Brick* brick,
                 const uint32_t nodeId)
{
    // get data
    Segment* segment = KyoukoRoot::m_segment;
    EdgeSection* edgeSection = &getBuffer<EdgeSection>(segment->edges)[nodeId];
    Brick* targetBrick = &getBuffer<Brick>(segment->bricks)[edgeSection->targetBrickId];

    // set vertizes
    result.vertizes.push_back(convertPos(brick->brickPos));
    result.vertizes.push_back(convertPos(targetBrick->brickPos));

    // create line
    const uint32_t actualPos = static_cast<uint32_t>(result.vertizes.size());
    result.lines.push_back({actualPos - 1, actualPos});

    // convert edges
    convertEdgesToObj(result, edgeSection, 1, actualPos);
}

//--------------------------------------------------------------------------------------------------

/**
 * @brief convertEdgesToObj
 * @param result
 * @param brick
 * @param node
 */
void
convertEdgesToObj(ObjItem &result,
                  EdgeSection* section,
                  const uint16_t pos,
                  const uint32_t vectorPos)
{
    cleanupEdgeSection(*section);

    Edge* edge = &section->edges[pos];
    if(getBrickId(edge->location) == 0x00FFFFFF) {
        return;
    }

    Segment* segment = KyoukoRoot::m_segment;
    std::cout<<"edge->currentBrickId: "<<getBrickId(edge->location)<<std::endl;
    Brick* brick = &getBuffer<Brick>(segment->bricks)[getBrickId(edge->location)];
    result.vertizes.push_back(convertPos(brick->brickPos));

    const uint32_t actualPos = static_cast<uint32_t>(result.vertizes.size());
    result.lines.push_back({vectorPos, actualPos});

    if(pos < 127)
    {
        convertEdgesToObj(result,
                          section,
                          pos * 2,
                          actualPos);

        convertEdgesToObj(result,
                          section,
                          (pos * 2) + 1,
                          actualPos);
    }
}

//--------------------------------------------------------------------------------------------------

}
