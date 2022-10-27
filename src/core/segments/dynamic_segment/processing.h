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
createNewSection(DynamicSegment &segment)
{
    SynapseSection newSection;
    newSection.active = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;
    newSection.randomPos = rand() % NUMBER_OF_RAND_VALUES;
    newSection.brickBufferPos = KyoukoRoot::m_randomValues[newSection.randomPos] % 1000;

    return segment.segmentData.addNewItem(newSection);
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
                 const DynamicNeuron &sourceNeuron,
                 const DynamicSegmentSettings &segmentSettings,
                 const float maxSegmentSize)
{
    float random = 0.0f;
    uint32_t targetNeuronIdInBrick = 0;
    Brick* targetBrick = nullptr;
    Brick* neuronBrick = nullptr;
    uint32_t signRand = 0;
    const uint32_t* randomValues = KyoukoRoot::m_randomValues;
    const float randMax = static_cast<float>(RAND_MAX);

    // set activation-border
    section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    random = static_cast<float>(randomValues[section.randomPos]) / randMax;
    synapse->border = (maxSegmentSize * 2.0f) * random;

    // set new weight
    synapse->weight = random / 10.0f;

    // update weight with sign
    section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    signRand = randomValues[section.randomPos] % 1000;
    synapse->weight *= static_cast<float>(1.0f - (1000.0f * segmentSettings.signNeg > signRand) * 2);

    // set target neuron
    section.randomPos = (section.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    neuronBrick = &bricks[sourceNeuron.brickId];
    targetBrick = &bricks[neuronBrick->possibleTargetNeuronBrickIds[section.brickBufferPos]];
    targetNeuronIdInBrick = randomValues[section.randomPos] % targetBrick->numberOfNeurons;

    synapse->targetNeuronId = static_cast<uint16_t>(targetNeuronIdInBrick + targetBrick->neuronPos);
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
                  DynamicSegment &segment,
                  const DynamicNeuron &sourceNeuron,
                  float netH,
                  const float maxSegmentSize)
{
    uint32_t pos = 0;
    Synapse* synapse = nullptr;
    DynamicNeuron* targetNeuron = nullptr;
    Synapse synapseObj;

    // iterate over all synapses in the section
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > maxSegmentSize)
    {
        synapse = &section.synapses[pos];
        synapseObj = *synapse;

        netH -= synapseObj.border;

        // break loop, if learning is disabled to the loop has reached an inactive synapse
        if(synapseObj.targetNeuronId == UNINIT_STATE_16) {
            break;
        }
        else if(synapseObj.targetNeuronId == 0)
        {
            pos++;
            continue;
        }

        // update target-neuron
        targetNeuron = &segment.neurons[synapseObj.targetNeuronId];
        targetNeuron->input += synapseObj.weight;

        // update loop-counter
        pos++;
    }

    if(netH > maxSegmentSize
            && section.next != UNINIT_STATE_32)
    {
        synapseProcessing(segment.synapseSections[section.next],
                          segment,
                          sourceNeuron,
                          netH,
                          maxSegmentSize);
    }
}

/**
 * @brief check the load on the section. If it would tigger not enough synapses inside the section
 *        then it is no good trained and should be cleared to try it again.
 *
 * @param section current processed synapse-section
 * @param netH wight-value, which comes into the section
 */
inline void
synapsePreprocessing(SynapseSection &section,
                     float netH)
{
    uint32_t pos = 0;

    // in case there are not enough interaction, delete section an try it again
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > 0.0f)
    {
        netH -= section.synapses[pos].border;
        pos++;
    }

    if(pos < 2)
    {
        for(uint16_t i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++) {
            section.synapses[i].targetNeuronId = UNINIT_STATE_16;
        }
    }
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
synapseProcessing_withLearn(SynapseSection &section,
                            DynamicSegment &segment,
                            const DynamicNeuron &sourceNeuron,
                            float netH,
                            const float maxSegmentSize)
{
    uint32_t pos = 0;
    Synapse* synapse = nullptr;
    DynamicNeuron* targetNeuron = nullptr;
    Synapse synapseObj;
    uint8_t active = 0;

    //synapsePreprocessing(section, netH);

    // iterate over all synapses in the section
    while(pos < SYNAPSES_PER_SYNAPSESECTION
          && netH > maxSegmentSize)
    {
        synapse = &section.synapses[pos];

        // create new synapse if necesarry and learning is active
        if(synapse->targetNeuronId == UNINIT_STATE_16)
        {
            createNewSynapse(section,
                             synapse,
                             segment.bricks,
                             sourceNeuron,
                             *segment.dynamicSegmentSettings,
                             maxSegmentSize);
        }

        synapseObj = *synapse;
        netH -= synapseObj.border;

        // break loop, if learning is disabled to the loop has reached an inactive synapse
        if(synapseObj.targetNeuronId == UNINIT_STATE_16) {
            break;
        }
        else if(synapseObj.targetNeuronId == 0)
        {
            pos++;
            continue;
        }

        // update target-neuron
        targetNeuron = &segment.neurons[synapseObj.targetNeuronId];
        targetNeuron->input += synapseObj.weight;

        // update active-counter
        active = (synapse->weight > 0) == (targetNeuron->potential > targetNeuron->border);
        synapse->activeCounter += active * static_cast<uint8_t>(synapseObj.activeCounter < 126);

        // update loop-counter
        pos++;
    }

    if(netH > maxSegmentSize)
    {
        // if no next section exist for the neuron, then create and a attach a new synapse-section
        if(section.next == UNINIT_STATE_32)
        {
            const uint64_t newPos = createNewSection(segment);
            // handle problem while allocating a new item for the section, for example if the
            // maximum number of items in the buffer is already in use
            if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                return;
            }

            section.next = newPos;
        }

        synapseProcessing_withLearn(segment.synapseSections[section.next],
                                    segment,
                                    sourceNeuron,
                                    netH,
                                    maxSegmentSize);
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
                    DynamicSegment &segment)
{
    // handle active-state
    if(neuron->active == 0) {
        return;
    }

    if(segment.dynamicSegmentSettings->doLearn > 0)
    {
        // if no target exist for the neuron, then create and a attach a new synapse-section
        if(neuron->targetSectionId == UNINIT_STATE_32)
        {
            const uint64_t newPos = createNewSection(segment);
            // handle problem while allocating a new item for the section, for example if the
            // maximum number of items in the buffer is already in use
            if(newPos == ITEM_BUFFER_UNDEFINE_POS) {
                return;
            }

            neuron->targetSectionId = newPos;
        }
    }

    if(neuron->targetSectionId == UNINIT_STATE_32) {
        return;
    }

    const float segm = static_cast<float>(segment.dynamicSegmentSettings->synapseSegmentation);
    const float maxSegmentSize = (neuron->potential / segm) / 2.0f;
    if(segment.dynamicSegmentSettings->doLearn > 0)
    {
        synapseProcessing_withLearn(segment.synapseSections[neuron->targetSectionId],
                                    segment,
                                    *neuron,
                                    neuron->potential,
                                    maxSegmentSize);
    }
    else
    {
        synapseProcessing(segment.synapseSections[neuron->targetSectionId],
                          segment,
                          *neuron,
                          neuron->potential,
                          maxSegmentSize);
    }
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

    // init border, if not set
    if(neuron->border == 0.00f) {
        neuron->border = neuron->input * 0.5f;
    }

    // handle refraction-time
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
 * @brief reset neurons of a transaction brick
 *
 * @param brick pointer to the brick
 * @param segment segment where the brick belongs to
 */
inline void
processNeuronsOfTransactionBrick(const Brick &brick,
                               const DynamicSegment &segment)
{
    DynamicNeuron* neuron = nullptr;

    for(uint32_t neuronId = brick.neuronPos;
        neuronId < brick.numberOfNeurons + brick.neuronPos;
        neuronId++)
    {
        neuron = &segment.neurons[neuronId];
        neuron->potential = segment.dynamicSegmentSettings->potentialOverflow * neuron->input;
        processNeuron(neuron, segment);
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

        processSingleNeuron(neuron, segment);
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
        processSingleNeuron(neuron, segment);
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
        } else if(brick->isTransactionBrick) {
            processNeuronsOfTransactionBrick(*brick, segment);
        } else {
            processNeuronsOfNormalBrick(*brick, segment);
        }
    }
}

#endif // KYOUKOMIND_DYNAMIC_PROCESSING_H
