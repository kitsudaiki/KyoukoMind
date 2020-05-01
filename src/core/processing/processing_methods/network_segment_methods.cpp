#include "network_segment_methods.h"

#include <core/processing/processing_methods/neighbor_methods.h>
#include <core/processing/processing_methods/brick_processing_methods.h>
#include <core/processing/processing_methods/brick_item_methods.h>
#include <core/processing/processing_methods/data_connection_methods.h>

namespace KyoukoMind
{


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
    SynapseSection* array = getSynapseSectionBlock(segment.synapses);
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

    Brick* bricks = getBrickBlock(segment);

    // set brick as output-brick
    bricks[brickPos].isOutputBrick = 1;

    // set the border-value of all nodes within the brick
    // to a high-value, so the node can never become active
    bricks[brickPos].nodeStart->border = 100000.0f;

    return true;
}

}
