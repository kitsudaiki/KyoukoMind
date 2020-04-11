#include <core/processing/processing_methods/brick_initializing_methods.h>

#include <core/processing/processing_methods/neighbor_methods.h>
#include <core/processing/processing_methods/brick_processing_methods.h>
#include <core/processing/processing_methods/brick_item_methods.h>

namespace KyoukoMind
{

//==================================================================================================

/**
 * initialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
initBrickNeighbor(Brick &sourceBrick,
                  const uint8_t sourceSide,
                  Brick* targetBrick,
                  Neighbor* targetNeighbor)
{

    // get and check neighbor
    Neighbor* neighbor = &sourceBrick.neighbors[sourceSide];
    assert(neighbor->inUse == 0);

    // init neighbor
    initNeighbor(*neighbor, targetBrick, targetNeighbor);

    return true;
}

//==================================================================================================

/**
 * uninitialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
uninitBrickNeighbor(Brick &sourceBrick,
                    const uint8_t side)
{
    // get and check neighbor
    Neighbor* neighbor = &sourceBrick.neighbors[side];
    assert(neighbor->inUse == 1);

    // uninit
    // TODO: issue #58
    neighbor->inUse = 0;

    return true;
}

//==================================================================================================

/**
 * connect two bricks by initialing the neighbors betweens the two bricks
 *
 * @return true, if successful, else false
 */
bool
connectBricks(Brick &sourceBrick,
              const uint8_t sourceSide,
              Brick &targetBrick)
{
    assert(sourceSide < 23);

    // get neighbor-pointers
    Neighbor* sourceNeighbor = &sourceBrick.neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick.neighbors[23-sourceSide];

    // check neighbors
    if(sourceNeighbor->inUse == 1
            || targetNeighbor->inUse == 1)
    {
        return false;
    }

    // init the new neighbors
    initBrickNeighbor(sourceBrick,
                      sourceSide,
                      &targetBrick,
                      targetNeighbor);
    initBrickNeighbor(targetBrick,
                      23 - sourceSide,
                      &sourceBrick,
                      sourceNeighbor);

    return true;
}

//==================================================================================================

/**
 * remove the connection between two neighbors
 *
 * @return true, if successful, else false
 */
bool
disconnectBricks(Brick &sourceBrick,
                 const uint8_t sourceSide,
                 Brick &targetBrick)
{
    assert(sourceSide < 23);

    // get neighbor-pointers
    Neighbor* sourceNeighbor = &sourceBrick.neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick.neighbors[23-sourceSide];

    // check neighbors
    if(sourceNeighbor->inUse == 0
            || targetNeighbor->inUse == 0)
    {
        return false;
    }

    // add the new neighbor
    uninitBrickNeighbor(sourceBrick, sourceSide);
    uninitBrickNeighbor(targetBrick, 23-sourceSide);

    return true;
}

//==================================================================================================

/**
 * initialize the node-list of the brick
 *
 * @return false if nodes are already initialized, esle true
*/
bool
initDataBlocks(Brick &brick,
               const uint8_t connectionId,
               const uint32_t numberOfItems,
               const uint32_t itemSize)
{
    DataConnection* data = &brick.dataConnections[connectionId];
    assert(itemSize != 0);
    assert(data->numberOfItems == 0);

    // update meta-data of the brick
    data->itemSize = itemSize;
    data->numberOfItems = numberOfItems;
    data->numberOfItemBlocks = (numberOfItems * data->itemSize) / data->buffer.blockSize + 1;

    // allocate blocks in buffer
    allocateBlocks(data->buffer, data->numberOfItemBlocks);

    return true;
}

//==================================================================================================

/**
 * initialize the node-list of the brick
 *
 * @return false if nodes are already initialized, esle true
 */
bool
initNodeBlocks(Brick &brick,
               uint32_t numberOfNodes)
{
    DataConnection* data = &brick.dataConnections[NODE_DATA];
    assert(data->numberOfItems == 0);

    // if not set by user, use default-value
    if(numberOfNodes == 0) {
        numberOfNodes = NUMBER_OF_NODES_PER_BRICK;
    }

    // init
    if(initDataBlocks(brick, NODE_DATA, numberOfNodes, sizeof(Node)) == false) {
        return false;
    }

    // fill array with empty nodes
    Node* array = static_cast<Node*>(data->buffer.data);
    for(uint16_t i = 0; i < numberOfNodes; i++)
    {
        Node tempNode;
        tempNode.border = (rand() % (MAXIMUM_NODE_BODER - MINIMUM_NODE_BODER)) + MINIMUM_NODE_BODER;
        array[i] = tempNode;
    }
    data->inUse = 1;

    return true;
}

//==================================================================================================

/**
 * init the edge-sections of thebrick
 *
 * @return false, if already initialized, else true
 */
bool
initSynapseSectionBlocks(Brick &brick,
                         const uint32_t numberOfSynapseSections)
{
    DataConnection* data = &brick.dataConnections[SYNAPSE_DATA];
    assert(data->inUse == 0);

    // init
    if(initDataBlocks(brick,
                      SYNAPSE_DATA,
                      numberOfSynapseSections,
                      sizeof(SynapseSection)) == false)
    {
        return false;
    }

    // fill array with empty synapsesections
    SynapseSection* array = getSynapseSectionBlock(data);
    for(uint32_t i = 0; i < numberOfSynapseSections; i++)
    {
        SynapseSection newSection;
        newSection.sourceId = i;
        array[i] = newSection;
    }
    data->inUse = 1;

    return true;
}

//==================================================================================================

/**
 * initialize forward-edge-block
 *
 * @return true if success, else false
 */
bool
initEdgeSectionBlocks(Brick &brick,
                      const uint32_t numberOfEdgeSections)
{
    DataConnection* data = &brick.dataConnections[EDGE_DATA];
    assert(data->inUse == 0);

    // init
    if(initDataBlocks(brick,
                      EDGE_DATA,
                      numberOfEdgeSections,
                      sizeof(EdgeSection)) == false)
    {
        return false;
    }

    // fill array with empty forward-edge-sections
    EdgeSection* array = getEdgeBlock(data);
    for(uint32_t i = 0; i < numberOfEdgeSections; i++)
    {
        // create new edge-section
        EdgeSection newSection;

        // connect all available sides
        for(uint8_t side = 0; side < 21; side++)
        {
            if(brick.neighbors[side].inUse != 0) {
                addEmptyEdge(newSection, side);
            }
        }

        array[i] = newSection;
    }

    data->inUse = 1;

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
addClientOutputConnection(Brick &brick)
{
    // get and check connection-item
    DataConnection* data = &brick.dataConnections[NODE_DATA];
    assert(data->inUse != 0);

    // set brick as output-brick
    brick.isOutputBrick = 1;

    // set the border-value of all nodes within the brick
    // to a high-value, so the node can never become active
    Node* start = static_cast<Node*>(data->buffer.data);
    Node* end = start + data->numberOfItems;
    for(Node* node = start;
        node < end;
        node++)
    {
        node->border = 100000.0f;
    }

    return true;
}

//==================================================================================================

/**
 * @brief initRandValues
 * @param brick
 */
void
initRandValues(Brick &brick)
{
    brick.randWeight = new float[999];
    float compare = 0.0f;
    for(uint32_t i = 0; i < 999; i++)
    {
        if(i % 3 == 0) {
            compare = 0.0f;
        }

        float tempValue = static_cast<float>(rand()) / 0x7FFFFFFF;
        assert(tempValue <= 1.0f);
        if(tempValue + compare > 1.0f) {
            tempValue = 1.0f - compare;
        }
        compare += tempValue;
        brick.randWeight[i] = tempValue;
    }

    brick.randValue = new uint32_t[1024];
    for(uint32_t i = 0; i < 1024; i++)
    {
        brick.randValue[i] = static_cast<uint32_t>(rand());
    }
}

//==================================================================================================

}
