﻿/**
 * @file        backpropagation.h
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

#ifndef KYOUKOMIND_DYNAMIC_BACKPROPAGATION_H
#define KYOUKOMIND_DYNAMIC_BACKPROPAGATION_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>
#include <core/segments/dynamic_segment/dynamic_segment.h>

#include "objects.h"

/**
 * @brief backpropagate values of an output-brick
 *
 * @param brick brick to process
 * @param segment segment where the brick belongs to
 */
inline bool
backpropagateOutput(const Brick* brick,
                    float* inputTransfers,
                    NeuronSection* neuronSections,
                    DynamicSegmentSettings* dynamicSegmentSettings)
{
    DynamicNeuron* neuron = nullptr;
    NeuronSection* section = nullptr;
    float totalDelta = 0.0f;

    // iterate over all neurons within the brick
    for(uint32_t neuronSectionId = brick->neuronSectionPos;
        neuronSectionId < brick->numberOfNeuronSections + brick->neuronSectionPos;
        neuronSectionId++)
    {
        section = &neuronSections[neuronSectionId];
        for(uint32_t neuronId = 0;
            neuronId < section->numberOfNeurons;
            neuronId++)
        {
            neuron = &section->neurons[neuronId];
            neuron->delta = inputTransfers[neuron->targetBorderId];
            inputTransfers[neuron->targetBorderId] = 0.0f;
            totalDelta += abs(neuron->delta);
        }
    }

    return totalDelta > dynamicSegmentSettings->backpropagationBorder;
    //return true;
}

/**
 * @brief run backpropagation for a single synapse-section
 *
 * @param section pointer to section to process
 * @param sourceNeuron pointer to the neuron, who triggered the section
 * @param netH neuron-potential
 * @param outH output-multiplicator
 * @param brick brick where the seciton is located
 * @param segment segment where section belongs to
 */
inline void
backpropagateSection(SynapseSection* section,
                     DynamicNeuron* sourceNeuron,
                     float netH,
                     const Brick* brick,
                     NeuronSection* neuronSections,
                     SynapseSection* synapseSections)
{
    Synapse* synapse = nullptr;
    DynamicNeuron* targetNeuron = nullptr;
    NeuronSection* neuronSection = &neuronSections[section->targetNeuronSectionId];
    float learnValue = 0.2f;
    uint16_t pos = 0;

    // iterate over all synapses in the section
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        // break look, if no more synapses to process
        synapse = &section->synapses[pos];

        // update weight
        learnValue = static_cast<float>(126 - synapse->activeCounter) * 0.0002f;
        learnValue += 0.05f;
        targetNeuron = &neuronSection->neurons[synapse->targetNeuronId];
        sourceNeuron->delta += targetNeuron->delta * synapse->weight;
        synapse->weight -= learnValue * targetNeuron->delta;

        netH -= synapse->border;
        pos++;
    }

    if(section->nextId != UNINIT_STATE_32
            && netH > 0.01f)
    {
        backpropagateSection(&synapseSections[section->nextId],
                             sourceNeuron,
                             netH,
                             brick,
                             neuronSections,
                             synapseSections);
    }
}

/**
 * @brief run back-propagation over the hidden neurons
 *
 * @param brick pointer to current brick
 * @param segment pointer to currect segment to process, which contains the brick
 */
inline void
backpropagateNeurons(const Brick* brick,
                     NeuronSection* neuronSections,
                     SynapseSection* synapseSections,
                     UpdatePosSection* updatePosSections,
                     float* outputTransfers)
{
    DynamicNeuron* sourceNeuron = nullptr;
    NeuronSection* neuronSection = nullptr;
    UpdatePosSection* updatePosSection = nullptr;

    // iterate over all neurons within the brick
    for(uint32_t neuronSectionId = brick->neuronSectionPos;
        neuronSectionId < brick->numberOfNeuronSections + brick->neuronSectionPos;
        neuronSectionId++)
    {
        neuronSection = &neuronSections[neuronSectionId];
        updatePosSection = &updatePosSections[neuronSectionId];
        for(uint32_t neuronId = 0;
            neuronId < neuronSection->numberOfNeurons;
            neuronId++)
        {
            // skip section, if not active
            sourceNeuron = &neuronSection->neurons[neuronId];
            //UpdatePos* updatePos = &updatePosSection->positions[neuronId];
            if(sourceNeuron->targetSectionId == UNINIT_STATE_32) {
                continue;
            }

            sourceNeuron->delta = 0.0f;

            // set start-values
            if(sourceNeuron->active)
            {
                backpropagateSection(&synapseSections[sourceNeuron->targetSectionId],
                                     sourceNeuron,
                                     sourceNeuron->potential,
                                     brick,
                                     neuronSections,
                                     synapseSections);

                sourceNeuron->delta *= 1.4427f * pow(0.5f, sourceNeuron->potential);
            }

            if(brick->isInputBrick) {
                outputTransfers[sourceNeuron->targetBorderId] = sourceNeuron->delta;
            }
        }
    }
}

/**
 * @brief correct wight of synapses within
 *
 * @param segment segment to process
 */
void
rewightDynamicSegment(const DynamicSegment &segment)
{
    Brick* bricks = segment.bricks;
    uint32_t* brickOrder = segment.brickOrder;
    NeuronSection* neuronSections = segment.neuronSections;
    SynapseSection* synapseSections = segment.synapseSections;
    SegmentHeader* segmentHeader = segment.segmentHeader;
    DynamicSegmentSettings* dynamicSegmentSettings = segment.dynamicSegmentSettings;
    UpdatePosSection* updatePosSections = segment.updatePosSections;
    float* inputTransfers = segment.inputTransfers;
    float* outputTransfers = segment.outputTransfers;

    // run back-propagation over all internal neurons and synapses
    const uint32_t numberOfBricks = segmentHeader->bricks.count;
    for(int32_t pos = numberOfBricks - 1; pos >= 0; pos--)
    {
        const uint32_t brickId = brickOrder[pos];
        Brick* brick = &bricks[brickId];
        if(brick->isOutputBrick)
        {
            if(backpropagateOutput(brick,
                                   inputTransfers,
                                   neuronSections,
                                   dynamicSegmentSettings) == false)
            {
                return;
            }
        }
        backpropagateNeurons(brick,
                             neuronSections,
                             synapseSections,
                             updatePosSections,
                             outputTransfers);
    }
}

#endif // KYOUKOMIND_DYNAMIC_BACKPROPAGATION_H
