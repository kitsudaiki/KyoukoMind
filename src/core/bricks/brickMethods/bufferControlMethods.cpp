/**
 *  @file    bufferControlMethods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "bufferControlMethods.h"
#include <core/messaging/messageObjects/contentContainer.h>
#include <core/messaging/messageMarker/outgoingBuffer.h>
#include <buffering/commonDataBufferMethods.h>

namespace KyoukoMind
{

/**
 * initialize the node-list of the brick
 *
 * @param numberOfItems number of new empty items
 * @return false if nodes are already initialized, esle true
*/
bool
initDataBlocks(Brick *brick,
               const uint8_t connectionId,
               const uint32_t numberOfItems,
               const uint32_t itemSize)
{
    DataConnection *data = &brick->dataConnections[connectionId];
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
    allocateBlocks(&data->buffer, data->numberOfItemBlocks);
    brick->updateBufferData();

    return true;
}

/**
 * initialize the node-list of the brick
 *
 * @param numberOfNodes number of new empty nodes
 * @return false if nodes are already initialized, esle true
 */
bool
initNodeBlocks(Brick* brick,
               uint32_t numberOfNodes)
{
    DataConnection* data = &brick->dataConnections[NODE_DATA];

    // prechecks
    if(data->numberOfItems != 0) {
        return false;
    }

    if(numberOfNodes == 0) {
        numberOfNodes = NUMBER_OF_NODES_PER_BRICK;
    }

    if(initDataBlocks(brick, NODE_DATA, numberOfNodes, sizeof(Node)) == false) {
        return false;
    }

    // fill array with empty nodes
    Node* array = (Node*)data->buffer.data;
    for(uint16_t i = 0; i < numberOfNodes; i++)
    {
        Node tempNode;
        tempNode.border = (rand() % (MAXIMUM_NODE_BODER - MINIMUM_NODE_BODER)) + MINIMUM_NODE_BODER;
        array[i] = tempNode;
    }
    data->inUse = 1;

    return true;
}

/**
 * init the edge-sections of thebrick
 *
 * @param numberOfEdgeSections number of edge-sections which should be initialized
 * @return false, if already initialized, else true
 */
bool
initEdgeSectionBlocks(Brick* brick,
                      const uint32_t numberOfEdgeSections)
{
    DataConnection* data = &brick->dataConnections[EDGE_DATA];

    // prechecks
    if(data->inUse != 0) {
        return false;
    }

    if(initDataBlocks(brick, EDGE_DATA, numberOfEdgeSections, sizeof(EdgeSection)) == false) {
        return false;
    }

    // fill array with empty edgesections
    EdgeSection* array = getEdgeSectionBlock(data);
    for(uint32_t i = 0; i < numberOfEdgeSections; i++)
    {
        EdgeSection newSection;
        newSection.sourceId = i;
        array[i] = newSection;
    }
    data->inUse = 1;

    return true;
}

/**
 * initialize forward-edge-block
 *
 * @param numberOfForwardEdgeSections number of forward-edge-sections
 * @return true if success, else false
 */
bool
initForwardEdgeSectionBlocks(Brick* brick,
                             const uint32_t numberOfForwardEdgeSections)
{
    DataConnection* data = &brick->dataConnections[FORWARDEDGE_DATA];

    // prechecks
    if(data->numberOfItems != 0
            || numberOfForwardEdgeSections == 0) {
        return false;
    }

    if(initDataBlocks(brick, FORWARDEDGE_DATA, numberOfForwardEdgeSections, sizeof(ForwardEdgeSection)) == false) {
        return false;
    }

    // fill array with empty forward-edge-sections
    ForwardEdgeSection* array = getForwardEdgeBlock(data);
    for(uint32_t i = 0; i < numberOfForwardEdgeSections; i++)
    {
        ForwardEdgeSection newSection;
        array[i] = newSection;
    }

    data->inUse = 1;

    return true;
}

/**
* @param itemPos
* @return
*/
bool
deleteDynamicItem(Brick* brick,
                  const uint8_t connectionId,
                  const uint32_t itemPos)
{
    DataConnection *data = &brick->dataConnections[connectionId];

    if(itemPos >= data->numberOfItems || data->inUse == 0) {
        return false;
    }

    // get buffer
    uint8_t* blockBegin = data->buffer.data;

    // overwrite item
    const uint32_t currentBytePos = itemPos * data->itemSize;
    EmptyPlaceHolder* placeHolder = (EmptyPlaceHolder*)&blockBegin[currentBytePos];

    if(placeHolder->status == DELETED_SECTION) {
        return false;
    }

    placeHolder->bytePositionOfNextEmptyBlock = UNINIT_STATE_32;
    placeHolder->status = DELETED_SECTION;

    // modify last place-holder
    if(data->bytePositionOfLastEmptyBlock != UNINIT_STATE_32)
    {
        EmptyPlaceHolder* lastPlaceHolder = (EmptyPlaceHolder*)&blockBegin[data->bytePositionOfLastEmptyBlock];
        lastPlaceHolder->bytePositionOfNextEmptyBlock = currentBytePos;
    }

    // set global values
    data->bytePositionOfLastEmptyBlock = currentBytePos;
    if(data->bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        data->bytePositionOfFirstEmptyBlock = currentBytePos;
    }

    data->numberOfDeletedDynamicItems++;

    return true;
}

/**
 * @brief reuseItemPosition
 * @param brick
 * @param connectionId
 * @return
 */
uint32_t
reuseItemPosition(Brick* brick,
                  const uint8_t connectionId)
{
    DataConnection *data = &brick->dataConnections[connectionId];

    const uint32_t selectedPosition = data->bytePositionOfFirstEmptyBlock;
    if(selectedPosition == UNINIT_STATE_32) {
        return UNINIT_STATE_32;
    }

    uint8_t* blockBegin = data->buffer.data;
    EmptyPlaceHolder* secetedPlaceHolder = (EmptyPlaceHolder*)&blockBegin[selectedPosition];
    data->bytePositionOfFirstEmptyBlock = secetedPlaceHolder->bytePositionOfNextEmptyBlock;

    if(data->bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        data->bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    }

    data->numberOfDeletedDynamicItems--;
    assert(selectedPosition % data->itemSize == 0);
    return selectedPosition / data->itemSize;
}

/**
* add a new forward-edge-section
*
* @return id of the new section, else SPECIAL_STATE if allocation failed
*/
uint32_t
reserveDynamicItem(Brick *brick,
                   const uint8_t connectionId)
{
    DataConnection *data = &brick->dataConnections[connectionId];

    if(data->itemSize == 0 || data->inUse == 0) {
        return UNINIT_STATE_32;
    }

    // try to reuse item
    const uint32_t reusePos = reuseItemPosition(brick, connectionId);
    if(reusePos != UNINIT_STATE_32) {
        return reusePos;
    }

    // allocate a new block, if necessary
    uint32_t blockSize = data->buffer.blockSize;
    if((data->numberOfItems * data->itemSize)/ blockSize
            < ((data->numberOfItems + 1) * data->itemSize) / blockSize)
    {
        if(!allocateBlocks(&data->buffer, 1)) {
            return UNINIT_STATE_32;
        }
        data->numberOfItemBlocks++;
    }

    data->numberOfItems++;
    return data->numberOfItems-1;
}

/**
 * add an existing edge to a specifig edge-sections
 *
 * @param edgeSectionId id of the edge-section for the new edge
 * @param newEdge new edge, which should be added
 * @return false, if edgeSectionId is too big, else true
 */
bool
addEdge(Brick* brick,
        const uint32_t edgeSectionId,
        const Edge &newEdge)
{
    const DataConnection* data = &brick->dataConnections[EDGE_DATA];

    // check if id is valid
    if(edgeSectionId >= data->numberOfItems) {
        return false;
    }

    // get section and add the new edge
    EdgeSection* edgeSection = &getEdgeSectionBlock(data)[edgeSectionId];
    return edgeSection->addEdge(newEdge);
}

/**
 * add a new empfy edge-section
 *
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
uint32_t
addEmptyEdgeSection(Brick* brick, const uint32_t sourceId)
{
    const uint32_t position = reserveDynamicItem(brick, EDGE_DATA);

    if(position != UNINIT_STATE_32)
    {
        // add new edge-forward-section
        EdgeSection newSection;
        newSection.sourceId = sourceId;
        const DataConnection* data = &brick->dataConnections[EDGE_DATA];
        getEdgeSectionBlock(data)[position] = newSection;
    }

    return position;
}

/**
 * add a new forward-edge-section
 *
 * @param sourceSide id of the incoming side
 * @param sourceId id of the source forward-edge-section
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
uint32_t
addEmptyForwardEdgeSection(Brick *brick,
                           const uint8_t sourceSide,
                           const uint32_t sourceId)
{
    const uint32_t position = reserveDynamicItem(brick, FORWARDEDGE_DATA);

    // add new edge-forward-section
    ForwardEdgeSection newSection;
    newSection.sourceId = sourceId;
    newSection.sourceSide = sourceSide;
    const DataConnection* connection = &brick->dataConnections[FORWARDEDGE_DATA];
    ForwardEdgeSection* array = getForwardEdgeBlock(connection);
    array[position] = newSection;

    return position;
}

}
