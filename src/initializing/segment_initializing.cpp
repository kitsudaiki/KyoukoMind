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

#include <libKitsunemimiCommon/buffer/data_buffer.h>

#include <core/objects/segment.h>
#include <core/objects/node.h>
#include <core/objects/output.h>
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
 * @brief initNodeBuffer
 * @param nodeBuffer
 * @param numberOfItems
 * @return
 */
bool
initNodeBuffer(ItemBuffer &nodeBuffer, const uint32_t numberOfItems)
{
    if(nodeBuffer.initBuffer<float>(numberOfItems) == false) {
        return false;
    }

    float* nodeProcessingBuffer = getBuffer<float>(nodeBuffer);
    for(uint32_t i = 0; i < numberOfItems; i++) {
        nodeProcessingBuffer[i] = 0.0f;
    }
    nodeBuffer.numberOfItems = numberOfItems;

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
    // init nodes itself
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

    // init node-buffer
    assert(initNodeBuffer(segment.nodeProcessingBuffer, numberOfNodes * 255));
    // TODO: correct number
    assert(initNodeBuffer(segment.nodeInputBuffer, KyoukoRoot::m_segment->numberOfNodesPerBrick));
    assert(initNodeBuffer(segment.nodeOutputBuffer, KyoukoRoot::m_segment->numberOfNodesPerBrick));

    return true;
}

bool
initRandomValues(Segment &segment)
{
    const uint32_t numberOfRandValues = 1024;
    if(segment.randomIntValues.initBuffer<uint32_t>(numberOfRandValues) == false) {
        return false;
    }

    uint32_t* randValue = getBuffer<uint32_t>(segment.randomIntValues);
    for(uint32_t i = 0; i < numberOfRandValues; i++) {
        randValue[i] = static_cast<uint32_t>(rand());
    }
    segment.randomIntValues.numberOfItems = numberOfRandValues;

    return true;
}

/**
 * init the edge-sections of thebrick
 *
 * @return false, if already initialized, else true
 */
bool
initSynapseSectionBlocks(Segment &segment,
                         const uint32_t numberOfSynapseSections,
                         const uint32_t numberOfNodes)
{
    assert(numberOfSynapseSections > 0);

    // init
    // segment.synapses.dynamic = true;
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

    // mark all synapses als delted to make them usable
    if(segment.synapses.deleteAll() == false) {
        return false;
    }

    Brick** nodeBricks = KyoukoRoot::m_segment->nodeBricks;
    for(uint32_t i = 0; i < numberOfNodes; i++)
    {
        array[i].status = ACTIVE_SECTION;
        array[i].randomPos = rand() % 1024;
        const uint32_t nodeBrickPos = rand() % KyoukoRoot::m_segment->numberOfNodeBricks;
        array[i].nodeBrickId = nodeBricks[nodeBrickPos]->nodeBrickId;
        assert(array[i].nodeBrickId < KyoukoRoot::m_segment->numberOfNodeBricks);
    }

    return true;
}

/**
 * @brief initOutputSynapses
 * @param segment
 * @param numberOfOutputBricks
 * @param numberOfOutputs
 * @return
 */
bool
initOutputSynapses(Segment &segment,
                   const uint32_t numberOfOutputBricks,
                   const uint32_t numberOfOutputs)
{
    const uint32_t outNodes = numberOfOutputBricks * KyoukoRoot::m_segment->numberOfNodesPerBrick;
    if(segment.outputSynapses.initBuffer<OutputSynapseSection>(outNodes) == false) {
        return false;
    }

    // fill array with empty output-sections
    OutputSynapseSection* outarray = getBuffer<OutputSynapseSection>(segment.outputSynapses);
    for(uint32_t i = 0; i < outNodes; i++)
    {
        OutputSynapseSection newSection;
        outarray[i] = newSection;
    }
    segment.outputSynapses.numberOfItems = outNodes;

    if(segment.outputs.initBuffer<Output>(numberOfOutputs) == false) {
        return false;
    }

    // fill array with empty output
    Output* outputArray = getBuffer<Output>(segment.outputs);
    for(uint32_t i = 0; i < numberOfOutputs; i++)
    {
        Output newSection;
        outputArray[i] = newSection;
    }
    segment.outputs.numberOfItems = numberOfOutputs;

    return true;
}
