/**
 * @file        section_update.h
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

#ifndef KYOUKOMIND_SECTION_UPDATE_H
#define KYOUKOMIND_SECTION_UPDATE_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>

#include "objects.h"
#include "dynamic_segment.h"

/**
 * @brief getBackwardLast
 * @param sourceId
 * @param sectionConnections
 * @return
 */
inline SynapseSection*
getBackwardLast(const uint32_t sourceId,
                SynapseSection* sectionConnections)
{
    SynapseSection* connection = &sectionConnections[sourceId];
    if(connection->backwardNext == UNINIT_STATE_32) {
        return connection;
    }

    return getBackwardLast(connection->backwardNext, sectionConnections);
}

/**
 * @brief getForwardLast
 * @param sourceId
 * @param sectionConnections
 * @return
 */
inline SynapseSection*
getForwardLast(const uint32_t sourceId,
               SynapseSection* sectionConnections)
{
    SynapseSection* connection = &sectionConnections[sourceId];
    if(connection->forwardNext == UNINIT_STATE_32) {
        return connection;
    }

    return getForwardLast(connection->forwardNext, sectionConnections);
}

/**
 * @brief add new basic synapse-section to segment
 *
 * @param segment refernce to segment
 *
 * @return position in buffer, where the section was added
 */
inline void
createNewSection(SynapseSection &result,
                 DynamicSegment &segment,
                 const Brick &currentBrick)
{
    result.active = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;
    result.randomPos = rand() % NUMBER_OF_RAND_VALUES;
    result.randomPos = (result.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    const uint32_t randVal = KyoukoRoot::m_randomValues[result.randomPos] % 1000;
    result.brickId = currentBrick.possibleTargetNeuronBrickIds[randVal];
    result.randomPos = (result.randomPos + 1) % NUMBER_OF_RAND_VALUES;
    result.targetNeuronSectionId = segment.bricks[result.brickId].neuronSectionPos;
    result.targetNeuronSectionId += KyoukoRoot::m_randomValues[result.randomPos]
                                     % segment.bricks[result.brickId].numberOfNeuronSections;

    uint32_t signRand = 0;
    const uint32_t* randomValues = KyoukoRoot::m_randomValues;
    const float randMax = static_cast<float>(RAND_MAX);
    const float sigNeg = segment.dynamicSegmentSettings->signNeg;

    // preinit all synapses within the section
    for(uint32_t i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++)
    {
        Synapse* synapse = &result.synapses[i];
        result.randomPos = (result.randomPos + 1) % NUMBER_OF_RAND_VALUES;
        synapse->weight = (static_cast<float>(randomValues[result.randomPos]) / randMax) / 10.0f;

        // update weight with sign
        result.randomPos = (result.randomPos + 1) % NUMBER_OF_RAND_VALUES;
        signRand = randomValues[result.randomPos] % 1000;
        synapse->weight *= static_cast<float>(1.0f - (1000.0f * sigNeg > signRand) * 2);
    }
}

/**
 * @brief processUpdatePositon_Cpu
 * @param segment
 * @param sectionId
 * @param sourceUpdatePos
 */
inline void
processUpdatePositon_Cpu(DynamicSegment &segment,
                         const uint32_t sectionId,
                         const uint32_t neuronId)
{
    NeuronSection* sourceSection = &segment.neuronSections[sectionId];
    Brick* currentBrick = &segment.bricks[sourceSection->brickId];

    SynapseSection newSection;
    createNewSection(newSection, segment, *currentBrick);
    const uint64_t newId = segment.segmentData.addNewItem(newSection);
    if(newId == ITEM_BUFFER_UNDEFINE_POS) {
        return;
    }

    DynamicNeuron* neuron = &sourceSection->neurons[neuronId];
    if(neuron->targetSectionId == UNINIT_STATE_32) {
        neuron->targetSectionId = newId;
    } else {
        getForwardLast(neuron->targetSectionId, segment.synapseSections)->forwardNext = newId;
    }

    NeuronSection* targetSection = &segment.neuronSections[newSection.targetNeuronSectionId];
    if(targetSection->backwardNextId == UNINIT_STATE_32) {
        targetSection->backwardNextId = newId;
    } else {
        getForwardLast(targetSection->backwardNextId, segment.synapseSections)->backwardNext = newId;
    }
}

/**
 * @brief updateSections
 * @param segment
 */
inline void
updateSections_Cpu(DynamicSegment &segment)
{
    UpdatePosSection* sourceUpdatePosSection = nullptr;
    UpdatePos* sourceUpdatePos = nullptr;

    // iterate over all neurons and add new synapse-section, if required
    for(uint32_t i = 0;
        i < segment.segmentHeader->updatePosSections.count;
        i++)
    {
        sourceUpdatePosSection = &segment.updatePosSections[i];
        for(uint32_t pos = 0;
            pos < sourceUpdatePosSection->numberOfPositions;
            pos++)
        {
            sourceUpdatePos = &sourceUpdatePosSection->positions[pos];
            if(sourceUpdatePos->type == 1)
            {
                sourceUpdatePos->type = 0;
                sourceUpdatePos->forwardNewId = UNINIT_STATE_32;
                processUpdatePositon_Cpu(segment, i, pos);
            }
        }
    }
}

/**
 * @brief updateSections
 * @param segment
 */
inline void
updateSections_Gpu(DynamicSegment &segment)
{
    UpdatePosSection* sourceUpdatePosSection = nullptr;
    UpdatePosSection* targetUpdatePosSection = nullptr;
    UpdatePos* updatePos = nullptr;
    NeuronSection* sourceSection = nullptr;
    UpdatePosSection* updatePosSections = segment.updatePosSections;
    Brick* currentBrick = nullptr;
    uint64_t newId = 0;

    // iterate over all neurons and add new synapse-section, if required
    for(uint32_t i = 0;
        i < segment.segmentHeader->updatePosSections.count;
        i++)
    {
        sourceUpdatePosSection = &updatePosSections[i];
        for(uint32_t pos = 0;
            pos < sourceUpdatePosSection->numberOfPositions;
            pos++)
        {
            updatePos = &sourceUpdatePosSection->positions[pos];
            if(updatePos->type == 1)
            {
                updatePos->type = 0;

                sourceSection = &segment.neuronSections[i];
                currentBrick = &segment.bricks[sourceSection->brickId];
                SynapseSection newSection;
                createNewSection(newSection, segment, *currentBrick);

                targetUpdatePosSection = &updatePosSections[newSection.targetNeuronSectionId];
                if(targetUpdatePosSection->backwardNewId != UNINIT_STATE_32) {
                    continue;
                }

                newId = segment.segmentData.addNewItem(newSection);
                if(newId == ITEM_BUFFER_UNDEFINE_POS)
                {
                    updatePos->forwardNewId = UNINIT_STATE_32;
                    return;
                }

                targetUpdatePosSection->backwardNewId = newId;
                updatePos->forwardNewId = newId;
                updatePos->randomPos = newSection.randomPos;
                updatePos->targetNeuronSectionId = newSection.targetNeuronSectionId;
            }
        }
    }
}

#endif // KYOUKOMIND_SECTION_UPDATE_H
