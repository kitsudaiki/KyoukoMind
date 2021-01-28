/**
 * @file        segment_initializing.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "segment_initializing.h"

#include <core/objects/segment.h>

#include <core/objects/transfer_objects.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

/**
 * @brief initBricks
 *
 * @param segment
 * @param numberOfBricks
 *
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

    segment.bricks.numberOfItems = numberOfBricks;

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
    const uint32_t numberOfNodesBuffer = numberOfNodes * 256;
    // init
    if(segment.nodes.initBuffer<Node>(numberOfNodesBuffer) == false) {
        return false;
    }

    // fill array with empty nodes
    Node* array = getBuffer<Node>(segment.nodes);
    for(uint32_t i = 0; i < numberOfNodesBuffer; i++)
    {
        Node tempNode;
        tempNode.border = (rand() % (MAXIMUM_NODE_BODER - MINIMUM_NODE_BODER)) + MINIMUM_NODE_BODER;
        array[i] = tempNode;
    }
    segment.nodes.numberOfItems = numberOfNodesBuffer;

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
