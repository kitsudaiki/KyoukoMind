/**
 * @file        segment.cpp
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

#include "segment.h"

#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>
#include <core/objects/synapses.h>
#include <core/objects/output.h>


Segment::Segment() {}

/**
 * @brief initNodeBuffer
 * @param nodeBuffer
 * @param numberOfItems
 * @return
 */
bool
Segment::initNodeBuffer(Kitsunemimi::ItemBuffer &nodeBuffer, const uint32_t numberOfItems)
{
    if(nodeBuffer.initBuffer<float>(numberOfItems) == false) {
        return false;
    }

    float* nodeProcessingBuffer = Kitsunemimi::getBuffer<float>(nodeBuffer);
    for(uint32_t i = 0; i < numberOfItems; i++) {
        nodeProcessingBuffer[i] = 0.0f;
    }

    return true;
}

/**
 * @brief Segment::initializeBuffer
 * @param numberOfBricks
 * @param numberOfNodeBricks
 * @param numberOfNodes
 * @param numberOfSynapseSections
 * @param numberOfOutputBricks
 * @param numberOfOutputs
 * @param numberOfRandValues
 * @return
 */
bool
Segment::initializeBuffer(const uint32_t numberOfBricks,
                          const uint32_t numberOfNodeBricks,
                          const uint32_t numberOfNodes,
                          const uint32_t numberOfSynapseSections,
                          const uint32_t numberOfOutputBricks,
                          const uint32_t numberOfOutputs,
                          const uint32_t numberOfRandValues)
{
    this->numberOfNodeBricks = numberOfNodeBricks;
    nodeBricks = new Brick*[numberOfNodeBricks];
    inputBricks = new Brick*[numberOfNodeBricks];
    outputBricks = new Brick*[numberOfNodeBricks];

    if(globalValues.initBuffer<GlobalValues>(1) == false) {
        return false;
    }

    if(bricks.initBuffer<Brick>(numberOfBricks) == false) {
        return false;
    }

    if(nodes.initBuffer<Node>(numberOfNodes) == false) {
        return false;
    }

    // init node-buffer
    // 255 == maximum number of threads on cpu or threads per block on gpu
    if(initNodeBuffer(nodeProcessingBuffer, numberOfNodes * 255) == false) {
        return false;
    }
    // TODO: correct number
    if(initNodeBuffer(nodeInputBuffer, nodesPerBrick) == false) {
        return false;
    }
    if(initNodeBuffer(nodeOutputBuffer, nodesPerBrick) == false) {
        return false;
    }

    if(randomIntValues.initBuffer<uint32_t>(numberOfRandValues) == false) {
        return false;
    }

    // segment.synapses.dynamic = true;
    if(synapses.initBuffer<SynapseSection>(numberOfSynapseSections) == false) {
        return false;
    }

    // mark all synapses als delted to make them usable
    for(uint64_t i = numberOfNodes; i < synapses.itemCapacity; i++) {
        synapses.deleteItem(i);
    }

    const uint32_t outNodes = numberOfOutputBricks * nodesPerBrick;
    if(outputSynapses.initBuffer<OutputSynapseSection>(outNodes) == false) {
        return false;
    }

    if(outputs.initBuffer<Output>(numberOfOutputs) == false) {
        return false;
    }

    return true;
}

/**
 * @brief BrickHandler::getMetadata
 * @return
 */
DataItem*
Segment::getMetadata()
{
    DataArray* edges = new DataArray();
    DataArray* nodes = new DataArray();

    // collect data
    for(uint32_t i = 0; i < bricks.itemCapacity; i++)
    {
        Brick* brick = &Kitsunemimi::getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[i];

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
