/**
 * @file        objects.h
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

#ifndef KYOUKOMIND_DYNAMIC_SEGMENT_OBJECTS_H
#define KYOUKOMIND_DYNAMIC_SEGMENT_OBJECTS_H

#include <common.h>
#include <libKitsunemimiCommon/buffer/item_buffer.h>

//==================================================================================================

struct DynamicNeuron
{
    float input = 0.0f;
    float border = 100.0f;
    float potential = 0.0f;
    float delta = 0.0f;

    uint8_t refractionTime = 1;
    uint8_t active = 0;
    uint8_t padding[2];

    uint32_t id = 0;

    uint32_t targetBorderId = UNINIT_STATE_32;
    uint32_t targetSectionId = UNINIT_STATE_32;

    // total size: 32 Byte
};

//==================================================================================================

struct NeuronSection
{
    DynamicNeuron neurons[NEURONS_PER_NEURONSECTION];
    uint32_t numberOfNeurons = 0;
    uint32_t id = 0;
    uint32_t brickId = 0;
    uint32_t backwardNextId = UNINIT_STATE_32;
    uint8_t padding[48];

    NeuronSection()
    {
        for(uint32_t i = 0; i < NEURONS_PER_NEURONSECTION; i++) {
            neurons[i] = DynamicNeuron();
        }
    }
    // total size: 2048 Byte
};

//==================================================================================================

struct Synapse
{
    float weight = 0.0f;
    float border = 0.0f;
    uint16_t targetNeuronId = UNINIT_STATE_16;
    int8_t activeCounter = 0;
    uint8_t padding[5];
    // total size: 16 Byte
};

//==================================================================================================

struct SynapseSection
{
    uint8_t active = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;
    uint8_t padding[3];
    uint32_t randomPos = 0;

    uint32_t targetNeuronSectionId = 0;
    uint32_t brickId = UNINIT_STATE_32;
    uint8_t padding2[8];
    uint32_t forwardNext = UNINIT_STATE_32;
    uint32_t backwardNext = UNINIT_STATE_32;

    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];

    SynapseSection()
    {
        for(uint32_t i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++) {
            synapses[i] = Synapse();
        }
    }
    // total size: 512 Byte
};

//==================================================================================================

struct UpdatePos
{
    uint32_t type = 0;
    uint32_t forwardNewId = UNINIT_STATE_32;
    // total size: 8 Byte
};

//==================================================================================================

struct UpdatePosSection
{
    UpdatePos positions[NEURONS_PER_NEURONSECTION];
    uint32_t numberOfPositions = 0;
    uint32_t backwardNewId = UNINIT_STATE_32;
    uint8_t padding[8];

    UpdatePosSection()
    {
        for(uint32_t i = 0; i < NEURONS_PER_NEURONSECTION; i++) {
            positions[i] = UpdatePos();
        }
    }
    // total size: 512 Byte
};

//==================================================================================================

struct DynamicSegmentSettings
{
    uint64_t maxSynapseSections = 0;
    float synapseDeleteBorder = 1.0f;
    float neuronCooldown = 100.0f;
    float memorizing = 0.1f;
    float gliaValue = 1.0f;
    float signNeg = 0.6f;
    float potentialOverflow = 1.0f;
    float synapseSegmentation = 10.0f;
    float backpropagationBorder = 0.00001f;
    uint8_t refractionTime = 1;
    uint8_t doLearn = 0;
    uint8_t updateSections = 0;

    uint8_t padding[213];

    // total size: 256 Byte
};

//==================================================================================================
#endif // KYOUKOMIND_DYNAMIC_SEGMENT_OBJECTS_H
