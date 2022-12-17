/**
 * @file        processing.h
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

#ifndef KYOUKOMIND_DYNAMIC_PROCESSING_H
#define KYOUKOMIND_DYNAMIC_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>

#include "objects.h"
#include "dynamic_segment.h"

/**
 * @brief add new basic synapse-section to segment
 *
 * @param segment refernce to segment
 *
 * @return position in buffer, where the section was added
 */
inline uint64_t
createNewSection(DynamicSegment &segment,
                 const Brick &currentBrick)
{
    SynapseSection section;
    section.active = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;
    section.randomPos = rand() % NUMBER_OF_RAND_VALUES;
    section.brickId = KyoukoRoot::m_randomValues[section.randomPos] % 1000;
    section.brickId = currentBrick.possibleTargetNeuronBrickIds[section.brickId];
    section.neuronOffset = segment.bricks[section.brickId].neuronPos;

    uint32_t signRand = 0;
    const uint32_t* randomValues = KyoukoRoot::m_randomValues;
    const float randMax = static_cast<float>(RAND_MAX);
    const float sigNeg = segment.dynamicSegmentSettings->signNeg;

    for(uint32_t i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++)
    {
        Synapse* synapse = &section.synapses[i];
        section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
        synapse->weight = (static_cast<float>(randomValues[section.randomPos]) / randMax) / 10.0f;

        // update weight with sign
        section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
        signRand = randomValues[section.randomPos] % 1000;
        synapse->weight *= static_cast<float>(1.0f - (1000.0f * sigNeg > signRand) * 2);
    }

    return segment.segmentData.addNewItem(section);
}

/**
 * @brief initialize a new specific synapse
 *
 * @param section current processed synapse-section
 * @param synapse new synapse, which has to be initialized
 * @param bricks array of all bricks
 * @param sourceNeuron source-neuron, who triggered the section
 * @param segmentSettings settings of the section
 * @param remainingWeight weight of which to cut of a part for the new synapse
 */
inline void
createNewSynapse(SynapseSection &section,
                 Synapse* synapse,
                 Brick* bricks,
                 const DynamicSegmentSettings &segmentSettings,
                 const float remainingWeight,
                 const float outH)
{
    const uint32_t* randomValues = KyoukoRoot::m_randomValues;
    const float randMax = static_cast<float>(RAND_MAX);
    const float maxWeight = outH / static_cast<float>(segmentSettings.synapseSegmentation);

    // set activation-border
    section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    float newWeight = maxWeight * (static_cast<float>(randomValues[section.randomPos]) / randMax);
    synapse->border = static_cast<float>(remainingWeight < newWeight) * remainingWeight
                      + static_cast<float>(remainingWeight >= newWeight) * newWeight;

    // set target neuron
    section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    synapse->targetNeuronId = static_cast<uint16_t>(randomValues[section.randomPos]
                                                    % bricks[section.brickId].numberOfNeurons);
    synapse->activeCounter = 1;
}

/**
 * @brief process synapse-section
 *
 * @param section current processed synapse-section
 * @param segment refernece to the processed segment
 * @param sourceNeuron source-neuron, who triggered the section
 * @param netH wight-value, which comes into the section
 * @param outH multiplicator
 */
inline void
synapseProcessing(SynapseSection &section,
                  const Brick &brick,
                  DynamicSegment &segment,
                  const DynamicNeuron &sourceNeuron,
                  float netH,
                  const float outH)
{
    uint32_t pos = 0;
    Synapse* synapse = nullptr;
    DynamicNeuron* targetNeuron = nullptr;
    //uint8_t active = 0;
    uint32_t nodePos = 0;

    //synapsePreprocessing(section, netH);

    // iterate over all synapses in the section
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        synapse = &section.synapses[pos];

        // create new synapse if necesarry and learning is active
        if(synapse->targetNeuronId == UNINIT_STATE_16)
        {
            createNewSynapse(section,
                             synapse,
                             segment.bricks,
                             *segment.dynamicSegmentSettings,
                             netH,
                             outH);
        }

        // update target-neuron
        nodePos = static_cast<uint32_t>(synapse->targetNeuronId) + section.neuronOffset;
        targetNeuron = &segment.neurons[nodePos];
        targetNeuron->input += synapse->weight;

        // update active-counter
        //active = (synapse->weight > 0) == (targetNeuron->potential > targetNeuron->border);
        //synapse->activeCounter += active * static_cast<uint8_t>(synapse->activeCounter < 126);

        // update loop-counter
        netH -= synapse->border;
        pos++;
    }

    if(netH > 0.01f)
    {
        // if no next section exist for the neuron, then create and a attach a new synapse-section
        if(section.next == UNINIT_STATE_32)
        {
            const uint64_t newPos = createNewSection(segment, brick);
            // handle problem while allocating a new item for the section, for example if the
            // maximum number of items in the buffer is already in use
            if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                return;
            }

            section.next = newPos;
        }

        synapseProcessing(segment.synapseSections[section.next],
                          brick,
                          segment,
                          sourceNeuron,
                          netH,
                          outH);
    }
}

/**
 * @brief process only a single neuron
 *
 * @param neuron pointer to neuron to process
 * @param segment segment where the neuron belongs to
 */
inline void
processSingleNeuron(DynamicNeuron* neuron,
                    const Brick &brick,
                    DynamicSegment &segment)
{
    // handle active-state
    if(neuron->active == 0) {
        return;
    }

    // if no target exist for the neuron, then create and a attach a new synapse-section
    if(neuron->targetSectionId == UNINIT_STATE_32)
    {
        const uint64_t newPos = createNewSection(segment, brick);
        // handle problem while allocating a new item for the section, for example if the
        // maximum number of items in the buffer is already in use
        if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
            return;
        }

        neuron->targetSectionId = newPos;
    }

    if(neuron->targetSectionId == UNINIT_STATE_32) {
        return;
    }

    synapseProcessing(segment.synapseSections[neuron->targetSectionId],
                      brick,
                      segment,
                      *neuron,
                      neuron->potential,
                      neuron->potential);
}

/**
 * @brief processNeuron
 * @param neuron
 * @param segment
 */
inline void
processNeuron(DynamicNeuron* neuron,
              const DynamicSegment &segment)
{
    neuron->potential /= segment.dynamicSegmentSettings->neuronCooldown;
    neuron->refractionTime = neuron->refractionTime >> 1;

    if(neuron->refractionTime == 0)
    {
        neuron->potential = segment.dynamicSegmentSettings->potentialOverflow * neuron->input;
        neuron->refractionTime = segment.dynamicSegmentSettings->refractionTime;
    }

    // update neuron
    neuron->potential -= neuron->border;
    neuron->active = neuron->potential > 0.0f;
    neuron->input = 0.0f;
    neuron->potential = log2(neuron->potential + 1.0f);
}

/**
 * @brief reset neurons of a output brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
processNeuronsOfOutputBrick(const Brick &brick,
                            const DynamicSegment &segment)
{
    DynamicNeuron* neuron = nullptr;

    for(uint32_t neuronId = brick.neuronPos;
        neuronId < brick.numberOfNeurons + brick.neuronPos;
        neuronId++)
    {
        neuron = &segment.neurons[neuronId];
        neuron->potential = segment.dynamicSegmentSettings->potentialOverflow * neuron->input;
        segment.outputTransfers[neuron->targetBorderId] = neuron->potential;
        neuron->input = 0.0f;
    }
}

/**
 * @brief reset neurons of a input brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
processNeuronsOfInputBrick(const Brick &brick,
                           DynamicSegment &segment)
{
    DynamicNeuron* neuron = nullptr;

    for(uint32_t neuronId = brick.neuronPos;
        neuronId < brick.numberOfNeurons + brick.neuronPos;
        neuronId++)
    {
        neuron = &segment.neurons[neuronId];
        neuron->potential = segment.inputTransfers[neuron->targetBorderId];
        neuron->active = neuron->potential > 0.0f;

        processSingleNeuron(neuron, brick, segment);
    }
}

/**
 * @brief reset neurons of a normal brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
processNeuronsOfNormalBrick(const Brick &brick,
                          DynamicSegment &segment)
{
    DynamicNeuron* neuron = nullptr;

    for(uint32_t neuronId = brick.neuronPos;
        neuronId < brick.numberOfNeurons + brick.neuronPos;
        neuronId++)
    {
        neuron = &segment.neurons[neuronId];
        processNeuron(neuron, segment);
        processSingleNeuron(neuron, brick, segment);
    }
}

/**
 * @brief process all neurons within a specific brick and also all synapse-sections,
 *        which are connected to an active neuron
 *
 * @param segment segment to process
 */
void
prcessDynamicSegment(DynamicSegment &segment)
{
    const uint32_t numberOfBricks = segment.segmentHeader->bricks.count;
    for(uint32_t pos = 0; pos < numberOfBricks; pos++)
    {
        const uint32_t brickId = segment.brickOrder[pos];
        Brick* brick = &segment.bricks[brickId];
        if(brick->isInputBrick) {
            processNeuronsOfInputBrick(*brick, segment);
        } else if(brick->isOutputBrick) {
            processNeuronsOfOutputBrick(*brick, segment);
        } else {
            processNeuronsOfNormalBrick(*brick, segment);
        }
    }
}

#endif // KYOUKOMIND_DYNAMIC_PROCESSING_H
