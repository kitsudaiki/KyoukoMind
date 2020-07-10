/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "network_segment.h"

#include <core/processing/objects/transfer_objects.h>
#include <core/processing/objects/node.h>
#include <core/processing/objects/synapses.h>

namespace KyoukoMind
{

NetworkSegment::NetworkSegment()
{

}

/**
 * @brief NetworkSegment::getNextTransferPos
 * @return
 */
uint32_t
NetworkSegment::getNextTransferPos()
{
    uint32_t pos = 0;

    while(lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    pos = synapseEdgesCounter;
    synapseEdgesCounter++;
    lock.clear(std::memory_order_release);

    return pos;
}

/**
 * @brief NetworkSegment::resetTransferPos
 */
void
NetworkSegment::resetTransferPos()
{
    while(lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    synapseEdgesCounter = 0;
    lock.clear(std::memory_order_release);
}

/**
 * @brief NetworkSegment::addEmptySynapseSection
 * @param sourceEdgeId
 * @param sourceBrickId
 * @return
 */
uint64_t
NetworkSegment::addEmptySynapseSection(const uint32_t sourceEdgeId,
                                       const uint32_t sourceBrickId)
{
    assert(sourceEdgeId != UNINIT_STATE_32);
    assert(sourceBrickId != UNINIT_STATE_32);

    const uint64_t position = synapses.reserveDynamicItem();
    assert(position != UNINIT_STATE_32);

    // add new edge-forward-section
    SynapseSection newSection;
    newSection.sourceEdgeId = sourceEdgeId;
    newSection.sourceBrickId = sourceBrickId;

    //getSynapseSectionBlock()[position] = newSection;

    return position;
}

/**
 * initialize the node-list of the brick
 *
 * @return false if nodes are already initialized, esle true
 */
bool
NetworkSegment::initNodeBlocks(const uint32_t &numberOfNodes)
{
    // init
    if(nodes.initDataBlocks(numberOfNodes,  sizeof(Node)) == false) {
        return false;
    }

    // fill array with empty nodes
    Node* array = static_cast<Node*>(nodes.buffer.data);
    for(uint32_t i = 0; i < numberOfNodes; i++)
    {
        Node tempNode;
        tempNode.border = (rand() % (MAXIMUM_NODE_BODER - MINIMUM_NODE_BODER)) + MINIMUM_NODE_BODER;
        array[i] = tempNode;
    }

    return true;
}

bool
NetworkSegment::initRandomValues()
{
    if(randomfloatValues.initDataBlocks(999,  sizeof(float)) == false) {
        return false;
    }

    float* randWeight = static_cast<float*>(randomfloatValues.buffer.data);
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
        randWeight[i] = tempValue;
    }

    if(randomIntValues.initDataBlocks(1024,  sizeof(uint32_t)) == false) {
        return false;
    }

    uint32_t* randValue = static_cast<uint32_t*>(randomIntValues.buffer.data);
    for(uint32_t i = 0; i < 1024; i++)
    {
        randValue[i] = static_cast<uint32_t>(rand());
    }

    return true;
}

/**
 * initialize forward-edge-block
 *
 * @return true if success, else false
 */
bool
NetworkSegment::initEdgeSectionBlocks(const uint32_t numberOfEdgeSections)
{
    // init
    if(edges.initDataBlocks(numberOfEdgeSections, sizeof(EdgeSection)) == false) {
        return false;
    }

    // fill array with empty forward-edge-sections
    EdgeSection* array = static_cast<EdgeSection*>(edges.buffer.data);
    for(uint32_t i = 0; i < numberOfEdgeSections; i++)
    {
        EdgeSection tempEdge;
        array[i] = tempEdge;
    }

    return true;
}

/**
 * init the edge-sections of thebrick
 *
 * @return false, if already initialized, else true
 */
bool
NetworkSegment::initSynapseSectionBlocks(const uint32_t numberOfSynapseSections)
{
    assert(numberOfSynapseSections > 0);

    // init
    if(synapses.initDataBlocks(numberOfSynapseSections, sizeof(SynapseSection)) == false) {
        return false;
    }

    // fill array with empty synapsesections
    SynapseSection* array = static_cast<SynapseSection*>(synapses.buffer.data);
    for(uint32_t i = 0; i < numberOfSynapseSections; i++)
    {
        SynapseSection newSection;
        array[i] = newSection;
    }

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
NetworkSegment::initTransferBlocks(const uint32_t totalNumberOfAxons,
                                   const uint64_t maxNumberOySynapseSections)
{
    //----------------------------------------------------------------------------------------------

    // init device-to-host-buffer
    if(axonEdges.initDataBlocks(totalNumberOfAxons, sizeof(AxonTransfer)) == false) {
        return false;
    }

    // fill array with empty values
    AxonTransfer* axonArray = static_cast<AxonTransfer*>(axonEdges.buffer.data);
    for(uint32_t i = 0; i < totalNumberOfAxons; i++)
    {
        AxonTransfer newEdge;
        axonArray[i] = newEdge;
    }

    //----------------------------------------------------------------------------------------------

    // init host-to-device-buffer
    if(synapseEdges.initDataBlocks(maxNumberOySynapseSections, sizeof(SynapseTransfer)) == false) {
        return false;
    }

    // fill array with empty values
    SynapseTransfer* synapseArray = static_cast<SynapseTransfer*>(synapseEdges.buffer.data);
    for(uint32_t i = 0; i < maxNumberOySynapseSections; i++)
    {
        SynapseTransfer newSynapseTransfer;
        synapseArray[i] = newSynapseTransfer;
    }

    //----------------------------------------------------------------------------------------------

    // init host-to-device-buffer
    if(updateEdges.initDataBlocks(maxNumberOySynapseSections,  sizeof(UpdateTransfer)) == false) {
        return false;
    }

    // fill array with empty values
    UpdateTransfer* updateArray = static_cast<UpdateTransfer*>(updateEdges.buffer.data);
    for(uint32_t i = 0; i < maxNumberOySynapseSections; i++)
    {
        UpdateTransfer newUpdateTransfer;
        updateArray[i] = newUpdateTransfer;
    }

    //----------------------------------------------------------------------------------------------

    return true;
}

/**
 * add a new client-connection to a brick,
 * for data input and output
 *
 * @return true, if successful, else false
 */
bool
NetworkSegment::addClientOutputConnection(const uint32_t brickPos)
{
    Brick* brick = bricks[brickPos];

    // set brick as output-brick
    brick->isOutputBrick = 1;

    // set the border-value of all nodes within the brick
    // to a high-value, so the node can never become active
    Node* nodeArray = &static_cast<Node*>(nodes.buffer.data)[brick->nodePos];
    nodeArray->border = 100000.0f;

    return true;
}

/**
 * @brief BrickHandler::getMetadata
 * @return
 */
DataItem*
NetworkSegment::getMetadata()
{
    DataArray* edges = new DataArray();
    DataArray* nodes = new DataArray();

    // collect data
    for(uint32_t i = 0; i < bricks.size(); i++)
    {
        Brick* brick = bricks[i];

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
NetworkSegment::connectBricks(const uint32_t sourceBrickId,
                              const uint8_t sourceSide,
                              const uint32_t targetBrickId)
{
    Brick* sourceBrick = bricks[sourceBrickId];
    Brick* targetBrick = bricks[targetBrickId];
    return sourceBrick->connectBricks(sourceSide, *targetBrick);
}

/**
 * disconnect two bricks from the handler from each other
 *
 * @return result of the sub-call
 */
bool
NetworkSegment::disconnectBricks(const uint32_t sourceBrickId,
                                 const uint8_t sourceSide,
                                 const uint32_t targetBrickId)
{
    Brick* sourceBrick = bricks[sourceBrickId];
    Brick* targetBrick = bricks[targetBrickId];
    return sourceBrick->disconnectBricks(sourceSide, *targetBrick);
}

}
