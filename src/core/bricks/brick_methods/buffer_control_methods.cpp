/**
 *  @file    bufferControlMethods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "buffer_control_methods.h"
#include <core/messaging/message_objects/content_container.h>
#include <core/messaging/message_buffer/outgoing_buffer.h>
#include <buffering/data_buffer_methods.h>

namespace KyoukoMind
{

/**
 * initialize the node-list of the brick
 *
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

    // if not set by user, use default-value
    if(numberOfNodes == 0) {
        numberOfNodes = NUMBER_OF_NODES_PER_BRICK;
    }

    // init
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
 * @return false, if already initialized, else true
 */
bool
initSynapseSectionBlocks(Brick* brick,
                         const uint32_t numberOfSynapseSections)
{
    DataConnection* data = &brick->dataConnections[SYNAPSE_DATA];

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

/**
 * initialize forward-edge-block
 *
 * @return true if success, else false
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

/**
* delete a specific item from the buffer by replacing it with a placeholder-item
*
* @return false if buffer is invalid or item already deleted, else true
*/
bool
deleteDynamicItem(Brick* brick,
                  const uint8_t connectionId,
                  const uint32_t itemPos)
{
    DataConnection *data = &brick->dataConnections[connectionId];

    // precheck
    if(itemPos >= data->numberOfItems
            || data->inUse == 0)
    {
        return false;
    }

    // get buffer
    uint8_t* blockBegin = static_cast<uint8_t*>(data->buffer.data);

    // data of the position
    const uint32_t currentBytePos = itemPos * data->itemSize;
    EmptyPlaceHolder* placeHolder = (EmptyPlaceHolder*)&blockBegin[currentBytePos];

    // check that the position is active and not already deleted
    if(placeHolder->status == DELETED_SECTION) {
        return false;
    }

    // overwrite item with a placeholder and set the position as delted
    placeHolder->bytePositionOfNextEmptyBlock = UNINIT_STATE_32;
    placeHolder->status = DELETED_SECTION;

    // modify last place-holder
    const uint32_t blockPosition = data->bytePositionOfLastEmptyBlock;
    if(blockPosition != UNINIT_STATE_32)
    {
        EmptyPlaceHolder* lastPlaceHolder = (EmptyPlaceHolder*)&blockBegin[blockPosition];
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
 * try to reuse a deleted buffer segment
 *
 * @return item-position in the buffer, else UNINIT_STATE_32 if no empty space in buffer exist
 */
uint32_t
reuseItemPosition(Brick* brick,
                  const uint8_t connectionId)
{
    DataConnection *data = &brick->dataConnections[connectionId];

    // get byte-position of free space, if exist
    const uint32_t selectedPosition = data->bytePositionOfFirstEmptyBlock;
    if(selectedPosition == UNINIT_STATE_32) {
        return UNINIT_STATE_32;
    }

    // set pointer to the next empty space
    uint8_t* blockBegin = static_cast<uint8_t*>(data->buffer.data);
    EmptyPlaceHolder* secetedPlaceHolder = (EmptyPlaceHolder*)&blockBegin[selectedPosition];
    data->bytePositionOfFirstEmptyBlock = secetedPlaceHolder->bytePositionOfNextEmptyBlock;

    // reset pointer, if no more free spaces exist
    if(data->bytePositionOfFirstEmptyBlock == UNINIT_STATE_32) {
        data->bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    }

    // convert byte-position to item-position and return this
    data->numberOfDeletedDynamicItems--;
    assert(selectedPosition % data->itemSize == 0);

    return selectedPosition / data->itemSize;
}

/**
* add a new forward-edge-section
*
* @return id of the new section, else UNINIT_STATE_32 if allocation failed
*/
uint32_t
reserveDynamicItem(Brick *brick,
                   const uint8_t connectionId)
{
    DataConnection* data = &brick->dataConnections[connectionId];

    // precheck
    if(data->itemSize == 0
            || data->inUse == 0)
    {
        return UNINIT_STATE_32;
    }

    // try to reuse item
    const uint32_t reusePos = reuseItemPosition(brick, connectionId);
    if(reusePos != UNINIT_STATE_32) {
        return reusePos;
    }

    // calculate size information
    const uint32_t blockSize = data->buffer.blockSize;
    const uint32_t currentNumberOfBlocks = (data->numberOfItems * data->itemSize)/ blockSize;
    const uint32_t newNumberOfBlocks = ((data->numberOfItems + 1) * data->itemSize) / blockSize;

    // allocate a new block, if necessary
    if(currentNumberOfBlocks < newNumberOfBlocks)
    {
        if(allocateBlocks(&data->buffer, 1) == false) {
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
 * @return false, if edgeSectionId is too big, else true
 */
bool
addSynapse(Brick* brick,
           const uint32_t synapseSectionId,
           const Synapse &newSynapse)
{
    const DataConnection* data = &brick->dataConnections[SYNAPSE_DATA];

    // check if id is valid
    if(synapseSectionId >= data->numberOfItems) {
        return false;
    }

    // get section and add the new edge
    SynapseSection* synapseSection = &getSynapseSectionBlock(data)[synapseSectionId];
    return synapseSection->addSynapse(newSynapse);
}

/**
 * add a new empfy edge-section
 *
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
uint32_t
addEmptySynapseSection(Brick* brick,
                       const uint32_t sourceId)
{
    const uint32_t position = reserveDynamicItem(brick, SYNAPSE_DATA);

    if(position != UNINIT_STATE_32)
    {
        // add new edge-forward-section
        SynapseSection newSection;
        newSection.sourceId = sourceId;
        const DataConnection* data = &brick->dataConnections[SYNAPSE_DATA];
        getSynapseSectionBlock(data)[position] = newSection;
    }

    return position;
}

/**
 * add a new edge-section to a specific brick with information about the source of the edge
 *
 * @return id of the new section, else SPECIAL_STATE if allocation failed
 */
uint32_t
addEmptyEdgeSection(Brick *brick,
                    const uint8_t sourceSide,
                    const uint32_t sourceId)
{
    const uint32_t position = reserveDynamicItem(brick, EDGE_DATA);

    // create new edge-section
    EdgeSection newSection;
    newSection.sourceId = sourceId;
    newSection.sourceSide = sourceSide;

    // add edge-section to the databuffer
    const DataConnection* connection = &brick->dataConnections[EDGE_DATA];
    EdgeSection* array = getEdgeBlock(connection);
    array[position] = newSection;

    return position;
}

} // namespace KyoukoMind
