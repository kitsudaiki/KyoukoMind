/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "segment.h"

#include <core/processing/objects/transfer_objects.h>
#include <core/processing/objects/node.h>
#include <core/processing/objects/synapses.h>
#include <core/object_handling/global_values.h>

#include <initializing/segment_initializing.h>

Segment::Segment()
{
    initGlobalValues(*this);
}

/**
 * add a new client-connection to a brick,
 * for data input and output
 *
 * @return true, if successful, else false
 */
bool
Segment::addClientOutputConnection(const uint32_t brickPos)
{
    Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[brickPos];

    // set brick as output-brick
    brick->isOutputBrick = 1;

    // set the border-value of all nodes within the brick
    // to a high-value, so the node can never become active
    Node* nodeArray = &getBuffer<Node>(nodes)[brick->nodePos];
    nodeArray->border = 100000.0f;

    return true;
}

/**
 * @brief BrickHandler::getMetadata
 * @return
 */
DataItem*
Segment::getMetadata()
{
    DataArray* edges = new DataArray();
    DataArray* nodes = new DataArray();

    // collect data
    for(uint32_t i = 0; i < bricks.itemCapacity; i++)
    {
        Brick* brick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[i];

        if(brick->nodePos != UNINIT_STATE_32) {
            nodes->append(new DataValue(static_cast<long>(brick->brickId)));
        }

        edges->append(new DataValue(static_cast<long>(brick->brickId)));
    }

    // build result
    DataMap* result = new DataMap();
    result->insert("edges", edges);
    result->insert("nodes", nodes);

    return result;
}

/**
 * @brief BrickHandler::connect
 * @param sourceBrickId
 * @param sourceSide
 * @param targetBrickId
 * @return
 */
bool
Segment::connectBricks(const uint32_t sourceBrickId,
                              const uint8_t sourceSide,
                              const uint32_t targetBrickId)
{
    Brick* sourceBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[sourceBrickId];
    Brick* targetBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[targetBrickId];
    return sourceBrick->connectBricks(sourceSide, *targetBrick);
}

/**
 * disconnect two bricks from the handler from each other
 *
 * @return result of the sub-call
 */
bool
Segment::disconnectBricks(const uint32_t sourceBrickId,
                                 const uint8_t sourceSide)
{
    Brick* sourceBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[sourceBrickId];
    return sourceBrick->disconnectBricks(sourceSide);
}
