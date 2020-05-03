#include "network_segment_methods.h"

#include <core/methods/neighbor_methods.h>
#include <core/methods/brick_processing_methods.h>
#include <core/methods/brick_item_methods.h>
#include <core/methods/brick_initializing_methods.h>
#include <core/methods/data_connection_methods.h>
#include <core/methods/network_segment_methods.h>

namespace KyoukoMind
{

//==================================================================================================

/**
 * initialize the node-list of the brick
 *
 * @return false if nodes are already initialized, esle true
 */
bool
initNodeBlocks(NetworkSegment &segment,
               uint32_t numberOfNodes)
{
    if(segment.nodes.numberOfItems != 0
            || segment.nodes.inUse != 0)
    {
        // TODO: log-output
        return false;
    }

    // if not set by user, use default-value
    if(numberOfNodes == 0) {
        numberOfNodes = NUMBER_OF_NODES_PER_BRICK;
    }

    // init
    if(initDataBlocks(segment.nodes,
                      numberOfNodes,
                      sizeof(Node)) == false)
    {
        return false;
    }

    // fill array with empty nodes
    Node* array = static_cast<Node*>(segment.nodes.buffer.data);
    for(uint16_t i = 0; i < numberOfNodes; i++)
    {
        Node tempNode;
        tempNode.border = (rand() % (MAXIMUM_NODE_BODER - MINIMUM_NODE_BODER)) + MINIMUM_NODE_BODER;
        array[i] = tempNode;
    }
    segment.nodes.inUse = 1;

    return true;
}

//==================================================================================================

/**
 * init the edge-sections of thebrick
 *
 * @return false, if already initialized, else true
 */
bool
initSynapseSectionBlocks(NetworkSegment &segment,
                         const uint32_t numberOfSynapseSections)
{
    assert(segment.synapses.inUse == 0);

    // init
    if(initDataBlocks(segment.synapses,
                      numberOfSynapseSections,
                      sizeof(SynapseSection)) == false)
    {
        return false;
    }

    // fill array with empty synapsesections
    SynapseSection* array = getSynapseSectionBlock(segment);
    for(uint32_t i = 0; i < numberOfSynapseSections; i++)
    {
        SynapseSection newSection;
        newSection.sourceId = i;
        array[i] = newSection;
    }
    segment.synapses.inUse = 1;

    return true;
}

//==================================================================================================

/**
 * add a new client-connection to a brick,
 * for data input and output
 *
 * @return true, if successful, else false
 */
bool
addClientOutputConnection(NetworkSegment &segment,
                          uint32_t brickPos)
{
    // get and check connection-item
    if(segment.nodes.inUse == 0)
    {
        // TODO: log-output
        return false;
    }

    Brick* brick = segment.bricks[brickPos];

    // set brick as output-brick
    brick->isOutputBrick = 1;

    // set the border-value of all nodes within the brick
    // to a high-value, so the node can never become active
    Node* nodeArray = &getNodeBlock(segment)[brick->nodePos];
    nodeArray->border = 100000.0f;

    return true;
}

//==================================================================================================

/**
 * @brief BrickHandler::getMetadata
 * @return
 */
Kitsunemimi::DataItem*
getMetadata(NetworkSegment &segment)
{
    DataArray* edges = new DataArray();
    DataArray* nodes = new DataArray();

    // collect data
    for(uint32_t i = 0; i < segment.bricks.size(); i++)
    {
        Brick* brick = segment.bricks[i];

        if(brick->nodePos >= 0) {
            nodes->append(new DataValue(static_cast<long>(brick->brickId)));
        }

        if(brick->edges.inUse != 0) {
            edges->append(new DataValue(static_cast<long>(brick->brickId)));
        }
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
connectBricks(NetworkSegment &segment,
              const BrickID sourceBrickId,
              const uint8_t sourceSide,
              const BrickID targetBrickId)
{
    Brick* sourceBrick = segment.bricks[sourceBrickId];
    Brick* targetBrick = segment.bricks[targetBrickId];
    return connectBricks(*sourceBrick, sourceSide, *targetBrick);
}

/**
 * disconnect two bricks from the handler from each other
 *
 * @return result of the sub-call
 */
bool
disconnectBricks(NetworkSegment &segment,
                 const BrickID sourceBrickId,
                 const uint8_t sourceSide,
                 const BrickID targetBrickId)
{
    Brick* sourceBrick = segment.bricks[sourceBrickId];
    Brick* targetBrick = segment.bricks[targetBrickId];
    return disconnectBricks(*sourceBrick, sourceSide, *targetBrick);
}

//==================================================================================================

}
