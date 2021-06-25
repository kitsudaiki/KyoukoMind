/**
 * @file        segment.h
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

#ifndef NETWORK_SEGMENT_H
#define NETWORK_SEGMENT_H

#include <common.h>

#include <libKitsunemimiCommon/buffer/item_buffer.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <libKitsunemimiAiCommon/metadata.h>

//==================================================================================================
struct SegmentHeaderEntry
{
    uint64_t bytePos = 0;
    uint64_t count = 0;

    // total size: 16 Byte
};

struct SegmentHeader
{
    // synapse-segment
    SegmentHeaderEntry settings;
    SegmentHeaderEntry bricks;
    SegmentHeaderEntry brickOrder;
    SegmentHeaderEntry synapseSections;
    SegmentHeaderEntry synapseBuffers;
    SegmentHeaderEntry nodes;
    SegmentHeaderEntry nodeBuffers;
    SegmentHeaderEntry inputs;
    SegmentHeaderEntry outputs;

    uint8_t padding[112];

    // total size: 256 Byte
};

struct Segment
{
    Kitsunemimi::DataBuffer buffer;

    SegmentHeader* segmentHeader = nullptr;
    Kitsunemimi::Ai::SegmentSettings* synapseMetaData = nullptr;
    Brick* bricks = nullptr;
    uint32_t* brickOrder = nullptr;
    Node* nodes = nullptr;
    float* nodeBuffers = nullptr;
    SynapseSection* synapseSections = nullptr;
    SynapseBuffer* synapseBuffers = nullptr;
    InputNode* inputs = nullptr;
    OutputNode* outputs = nullptr;

    Segment() {}
};

inline void
initSegmentPointer(Segment &segment)
{
    uint8_t* data = static_cast<uint8_t*>(segment.buffer.data);

    segment.segmentHeader = reinterpret_cast<SegmentHeader*>(data + 0);
    segment.synapseMetaData = reinterpret_cast<Kitsunemimi::Ai::SegmentSettings*>(data + 256);
    segment.bricks = reinterpret_cast<Brick*>(data + segment.segmentHeader->bricks.bytePos);
    segment.brickOrder = reinterpret_cast<uint32_t*>(data + segment.segmentHeader->brickOrder.bytePos);
    segment.nodes = reinterpret_cast<Node*>(data + segment.segmentHeader->nodes.bytePos);
    segment.nodeBuffers = reinterpret_cast<float*>(data + segment.segmentHeader->nodeBuffers.bytePos);
    segment.synapseSections = reinterpret_cast<SynapseSection*>(data + segment.segmentHeader->synapseSections.bytePos);
    segment.synapseBuffers = reinterpret_cast<SynapseBuffer*>(data + segment.segmentHeader->synapseBuffers.bytePos);
    segment.inputs = reinterpret_cast<InputNode*>(data + segment.segmentHeader->inputs.bytePos);
    segment.outputs = reinterpret_cast<OutputNode*>(data + segment.segmentHeader->outputs.bytePos);
}

//==================================================================================================

#endif // NETWORK_SEGMENT_H
