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
#include <core/objects/output.h>
#include <libKitsunemimiAiCommon/metadata.h>

enum SegmentType
{
    UNDEFINED_SEGMENT = 0,
    SYNAPSE_SEGMENT = 1,
    OUTPUT_SEGMENT = 2
};

//==================================================================================================

struct SynapseSegmentMeta
{
    uint32_t segmentType = UNDEFINED_SEGMENT;

    // synapse-segment
    uint32_t numberOfNodeBricks = 0;
    uint64_t numberOfSynapseSections = 0;
    uint64_t numberOfNodes = 0;
    uint32_t numberOfNodesPerBrick = 0;

    // generic
    uint32_t numberOfRandomValues = 0;
    uint32_t numberOfInputs = 0;

    uint8_t padding[220];
};

struct SynapseSegment
{
    Kitsunemimi::DataBuffer buffer;

    // generic objects
    SynapseSegmentMeta* segmentMeta = nullptr;
    Kitsunemimi::Ai::SynapseMetaData* synapseMetaData = nullptr;
    uint32_t* randomValues = nullptr;

    // bricks
    Brick* nodeBricks = nullptr;

    // nodes
    Node* nodes = nullptr;
    float* nodeBuffers = nullptr;

    // synapses
    SynapseSection* synapseSections = nullptr;
    SynapseBuffer* synapseBuffers = nullptr;

    InputNode* inputNodes = nullptr;

    std::vector<std::vector<Brick*>> layer;

    SynapseSegment() {}

};

//==================================================================================================

struct OutputSegmentMeta
{
    uint32_t segmentType = UNDEFINED_SEGMENT;

    // output-segment
    uint32_t numberOfOutputs = 0;

    // generic
    uint32_t numberOfRandomValues = 0;
    uint32_t numberOfInputs = 0;

    uint8_t padding[240];
};

struct OutputSegment
{
    Kitsunemimi::DataBuffer buffer;

    // generic objects
    OutputSegmentMeta* segmentMeta = nullptr;
    Kitsunemimi::Ai::OutputMetaData* outputMetaData = nullptr;
    uint32_t* randomValues = nullptr;

    // output
    Output* outputs = nullptr;
    OutputSynapseSection* outputSynapseSections = nullptr;


    OutputInput* inputs = nullptr;

    OutputSegment() {}

};

//==================================================================================================

#endif // NETWORK_SEGMENT_H
