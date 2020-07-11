/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "network_segment.h"

#include <core/processing/objects/transfer_objects.h>
#include <core/processing/objects/node.h>
#include <core/processing/objects/synapses.h>
#include <core/global_values.h>

namespace KyoukoMind
{

NetworkSegment::NetworkSegment()
{
    initGlobalValues();
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

    SynapseSection newSection;
    newSection.sourceEdgeId = sourceEdgeId;
    newSection.sourceBrickId = sourceBrickId;

    const uint64_t position = synapses.addNewItem(newSection);

    return position;
}

/**
 * @brief NetworkSegment::initBricks
 * @param numberOfBricks
 * @return
 */
bool
NetworkSegment::initBricks(const uint32_t numberOfBricks)
{
    // init
    if(bricks.initBuffer<Brick>(numberOfBricks) == false) {
        return false;
    }

    // fill array with empty nodes
    Brick* array = static_cast<Brick*>(bricks.buffer.data);
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
NetworkSegment::initGlobalValues()
{
    // init
    if(globalValues.initBuffer<GlobalValues>(1) == false) {
        return false;
    }

    // fill array with empty nodes
    GlobalValues* array = static_cast<GlobalValues*>(globalValues.buffer.data);
    GlobalValues tempValues;
    array[0] = tempValues;

    return true;
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
    if(nodes.initBuffer<Node>(numberOfNodes) == false) {
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
    if(randomfloatValues.initBuffer<float>(999) == false) {
        return false;
    }

    float* randWeight = getBuffer<float>(randomfloatValues);
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

    if(randomIntValues.initBuffer<uint32_t>(1024) == false) {
        return false;
    }

    uint32_t* randValue = getBuffer<uint32_t>(randomIntValues);
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
    if(edges.initBuffer<EdgeSection>(numberOfEdgeSections) == false) {
        return false;
    }

    // fill array with empty forward-edge-sections
    EdgeSection* array = getBuffer<EdgeSection>(edges);
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
    if(synapses.initBuffer<SynapseSection>(numberOfSynapseSections) == false) {
        return false;
    }

    // fill array with empty synapsesections
    SynapseSection* array = getBuffer<SynapseSection>(synapses);
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
    if(axonTransfers.initBuffer<AxonTransfer>(totalNumberOfAxons) == false) {
        return false;
    }

    // fill array with empty values
    AxonTransfer* axonArray = getBuffer<AxonTransfer>(axonTransfers);
    for(uint32_t i = 0; i < totalNumberOfAxons; i++)
    {
        AxonTransfer newEdge;
        axonArray[i] = newEdge;
    }

    //----------------------------------------------------------------------------------------------

    // init host-to-device-buffer
    if(synapseTransfers.initBuffer<SynapseTransfer>(maxNumberOySynapseSections) == false) {
        return false;
    }

    // fill array with empty values
    SynapseTransfer* synapseArray = getBuffer<SynapseTransfer>(synapseTransfers);
    for(uint32_t i = 0; i < maxNumberOySynapseSections; i++)
    {
        SynapseTransfer newSynapseTransfer;
        synapseArray[i] = newSynapseTransfer;
    }

    //----------------------------------------------------------------------------------------------

    // init host-to-device-buffer
    if(updateTransfers.initBuffer<UpdateTransfer>(maxNumberOySynapseSections) == false) {
        return false;
    }

    // fill array with empty values
    UpdateTransfer* updateArray = getBuffer<UpdateTransfer>(updateTransfers);
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
NetworkSegment::getMetadata()
{
    DataArray* edges = new DataArray();
    DataArray* nodes = new DataArray();

    // collect data
    for(uint32_t i = 0; i < bricks.numberOfItems; i++)
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
NetworkSegment::connectBricks(const uint32_t sourceBrickId,
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
NetworkSegment::disconnectBricks(const uint32_t sourceBrickId,
                                 const uint8_t sourceSide)
{
    Brick* sourceBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[sourceBrickId];
    return sourceBrick->disconnectBricks(sourceSide);
}

}
