#include "segment_initializing.h"

#include <core/object_handling/segment.h>

#include <core/processing/objects/transfer_objects.h>
#include <core/processing/objects/node.h>
#include <core/processing/objects/synapses.h>
#include <core/object_handling/global_values.h>

namespace KyoukoMind
{

/**
 * @brief NetworkSegment::initBricks
 * @param numberOfBricks
 * @return
 */
bool
initBricks(Segment &segment,
           const uint32_t numberOfBricks)
{
    // init
    if(segment.bricks.initBuffer<Brick>(numberOfBricks) == false) {
        return false;
    }

    // fill array with empty nodes
    Brick* array = getBuffer<Brick>(segment.bricks);
    for(uint32_t i = 0; i < numberOfBricks; i++)
    {
        Brick tempBrick;
        array[i] = tempBrick;
    }

    return true;
}

/**
 * @brief NetworkSegment::initGlobalValues
 * @return
 */
bool
initGlobalValues(Segment &segment)
{
    // init
    if(segment.globalValues.initBuffer<GlobalValues>(1) == false) {
        return false;
    }

    // fill array with empty nodes
    GlobalValues* array = getBuffer<GlobalValues>(segment.globalValues);
    GlobalValues tempValues;
    array[0] = tempValues;
    segment.globalValues.numberOfItems = 1;

    return true;
}

/**
 * initialize the node-list of the brick
 *
 * @return false if nodes are already initialized, esle true
 */
bool
initNodeBlocks(Segment &segment,
               const uint32_t &numberOfNodes)
{
    // init
    if(segment.nodes.initBuffer<Node>(numberOfNodes) == false) {
        return false;
    }

    // fill array with empty nodes
    Node* array = getBuffer<Node>(segment.nodes);
    for(uint32_t i = 0; i < numberOfNodes; i++)
    {
        Node tempNode;
        tempNode.border = (rand() % (MAXIMUM_NODE_BODER - MINIMUM_NODE_BODER)) + MINIMUM_NODE_BODER;
        array[i] = tempNode;
    }
    segment.nodes.numberOfItems = numberOfNodes;

    return true;
}

bool
initRandomValues(Segment &segment)
{
    if(segment.randomIntValues.initBuffer<uint32_t>(1024) == false) {
        return false;
    }

    uint32_t* randValue = getBuffer<uint32_t>(segment.randomIntValues);
    for(uint32_t i = 0; i < 1024; i++)
    {
        randValue[i] = static_cast<uint32_t>(rand());
    }
    segment.randomIntValues.numberOfItems = 1024;

    return true;
}

/**
 * initialize forward-edge-block
 *
 * @return true if success, else false
 */
bool
initEdgeSectionBlocks(Segment &segment,
                      const uint32_t numberOfEdgeSections)
{
    // init
    if(segment.edges.initBuffer<EdgeSection>(numberOfEdgeSections) == false) {
        return false;
    }

    // fill array with empty forward-edge-sections
    EdgeSection* array = getBuffer<EdgeSection>(segment.edges);
    for(uint32_t i = 0; i < numberOfEdgeSections; i++)
    {
        EdgeSection tempEdge;
        tempEdge.randomPos = i % 1024;
        array[i] = tempEdge;
    }
    segment.edges.numberOfItems = numberOfEdgeSections;

    return true;
}

/**
 * init the edge-sections of thebrick
 *
 * @return false, if already initialized, else true
 */
bool
initSynapseSectionBlocks(Segment &segment,
                         const uint32_t numberOfSynapseSections)
{
    assert(numberOfSynapseSections > 0);

    // init
    segment.synapses.dynamic = true;
    if(segment.synapses.initBuffer<SynapseSection>(numberOfSynapseSections) == false) {
        return false;
    }

    // fill array with empty synapsesections
    SynapseSection* array = getBuffer<SynapseSection>(segment.synapses);
    for(uint32_t i = 0; i < numberOfSynapseSections; i++)
    {
        SynapseSection newSection;
        array[i] = newSection;
    }
    segment.synapses.numberOfItems = numberOfSynapseSections;

    return true;
}

/**
 * @brief initTransferBlocks
 * @param segment
 * @param totalNumberOfAxons
 * @param maxNumberOySynapseSections
 * @return
 */
bool
initTransferBlocks(Segment &segment,
                   const uint32_t totalNumberOfAxons,
                   const uint64_t maxNumberOySynapseSections)
{
    //----------------------------------------------------------------------------------------------

    // init device-to-host-buffer
    if(segment.axonTransfers.initBuffer<AxonTransfer>(totalNumberOfAxons) == false) {
        return false;
    }

    // fill array with empty values
    AxonTransfer* axonArray = getBuffer<AxonTransfer>(segment.axonTransfers);
    for(uint32_t i = 0; i < totalNumberOfAxons; i++)
    {
        AxonTransfer newEdge;
        axonArray[i] = newEdge;
    }
    segment.axonTransfers.numberOfItems = totalNumberOfAxons;

    //----------------------------------------------------------------------------------------------

    // init host-to-device-buffer
    if(segment.synapseTransfers.initBuffer<SynapseTransfer>(maxNumberOySynapseSections) == false) {
        return false;
    }

    // fill array with empty values
    SynapseTransfer* synapseArray = getBuffer<SynapseTransfer>(segment.synapseTransfers);
    for(uint32_t i = 0; i < maxNumberOySynapseSections; i++)
    {
        SynapseTransfer newSynapseTransfer;
        synapseArray[i] = newSynapseTransfer;
    }
    segment.synapseTransfers.numberOfItems = maxNumberOySynapseSections;

    //----------------------------------------------------------------------------------------------

    // init host-to-device-buffer
    if(segment.updateTransfers.initBuffer<UpdateTransfer>(maxNumberOySynapseSections) == false) {
        return false;
    }

    // fill array with empty values
    UpdateTransfer* updateArray = getBuffer<UpdateTransfer>(segment.updateTransfers);
    for(uint32_t i = 0; i < maxNumberOySynapseSections; i++)
    {
        UpdateTransfer newUpdateTransfer;
        updateArray[i] = newUpdateTransfer;
    }
    segment.updateTransfers.numberOfItems = maxNumberOySynapseSections;

    //----------------------------------------------------------------------------------------------

    return true;
}

}
