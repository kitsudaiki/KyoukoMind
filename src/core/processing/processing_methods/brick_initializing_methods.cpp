#include <core/processing/processing_methods/brick_initializing_methods.h>

namespace KyoukoMind
{

//==================================================================================================

/**
 * initialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
initNeighbor(Brick &brick,
             const uint8_t sourceSide,
             const uint32_t targetBrickId)
{

    // get and check neighbor
    Neighbor* neighbor = &brick.neighbors[sourceSide];
    if(neighbor->inUse == 1) {
        return false;
    }

    // update ready-mask
    uint32_t pos = 0x1;
    brick.readyMask = brick.readyMask + (pos << sourceSide);

    // init neighbo
    initNeighbor(*neighbor,
                 sourceSide,
                 targetBrickId);
    updateBufferData(brick);

    return true;
}

//==================================================================================================

/**
 * uninitialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
uninitNeighbor(Brick &brick,
               const uint8_t side)
{
    // get and check neighbor
    Neighbor* neighbor = &brick.neighbors[side];
    if(neighbor->inUse == 0) {
        return false;
    }

    // update ready-mask
    uint32_t pos = 0x1;
    brick.readyMask = brick.readyMask - (pos << side);

    // uninit
    // TODO: issue #58
    neighbor->inUse = 0;

    // write brick-metadata to buffer
    updateBufferData(brick);

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
    // check if side is valid
    if(sourceSide >= 24) {
        return false;
    }

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
    initNeighbor(sourceBrick, sourceSide, targetBrick.brickId);
    initNeighbor(targetBrick, 23-sourceSide, sourceBrick.brickId);

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
    // check if side is valid
    if(sourceSide >= 24) {
        return false;
    }

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
    uninitNeighbor(sourceBrick, sourceSide);
    uninitNeighbor(targetBrick, 23-sourceSide);

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
    DataConnection *data = &brick.dataConnections[connectionId];

    // prechecks
    if(data->numberOfItems != 0
            || itemSize == 0) {
        return false;
    }

    // update meta-data of the brick
    data->itemSize = itemSize;
    data->numberOfItems = numberOfItems;
    data->numberOfItemBlocks = (numberOfItems * data->itemSize) / data->buffer.blockSize + 1;

    // allocate blocks in buffer
    allocateBlocks(data->buffer, data->numberOfItemBlocks);
    updateBufferData(brick);

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

    // prechecks
    if(data->numberOfItems != 0) {
        return false;
    }

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

    // prechecks
    if(data->inUse != 0) {
        return false;
    }

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

    // prechecks
    if(data->inUse != 0) {
        return false;
    }

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
        EdgeSection newSection;
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
addClientConnection(Brick &brick,
                    const bool isInput,
                    const bool isOutput)
{
    // add input-connection
    if(isInput)
    {
        // set brick as input-brick
        brick.isInputBrick = 1;

        // init the incoming-message-queue
        // for incoming messages from the client
        initNeighbor(brick, 24, UNINIT_STATE_32);
    }

    // add output-connection
    if(isOutput)
    {
        // get and check connection-item
        DataConnection* data = &brick.dataConnections[NODE_DATA];
        if(data->inUse == 0) {
            return false;
        }

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
    }

    return true;
}

}
