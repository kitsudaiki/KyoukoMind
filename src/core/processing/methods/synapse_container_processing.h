/**
 *  @file    message_processing_methods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef SYNAPSE_CONTAINER_PROCESSING_H
#define SYNAPSE_CONTAINER_PROCESSING_H

#include <common.h>
#include <root_object.h>

#include <core/objects/container_definitions.h>
#include <core/methods/brick_item_methods.h>
#include <core/methods/synapse_methods.h>
#include <core/methods/data_connection_methods.h>
#include <core/methods/network_segment_methods.h>

#include <core/objects/brick.h>
#include <core/objects/network_segment.h>

namespace KyoukoMind
{

//==================================================================================================

/**
 * @brief NodeBrick::createNewEdge
 * @param currentSection
 * @param edgeSectionId
 */
inline void
createNewSynapse(Brick &brick,
                 SynapseSection &currentSection)
{
    const uint32_t targetNodeId = brick.randValue[brick.randValuePos];
    brick.randValuePos = (brick.randValuePos + 1) % 1024;
    const uint32_t somaDistance = brick.randValue[brick.randValuePos];
    brick.randValuePos = (brick.randValuePos + 1) % 1024;

    addSynapse(currentSection,
               brick.globalValues.globalMemorizingOffset,
               targetNodeId,
               somaDistance);
}

//==================================================================================================

/**
 * learing-process of the specific synapse-section
 *
 * @param currentSection synapse-section with should learn the new value
 * @param weight weight-difference to learn
 */
inline void
learningSynapseSection(Brick &brick,
                       SynapseSection &currentSection,
                       float weight)
{
    if(weight < NEW_SYNAPSE_BORDER) {
        return;
    }

    for(uint8_t i = 0; i < 3; i++)
    {
        const uint32_t choosePosition = brick.randValue[brick.randValuePos]
                                        % (currentSection.numberOfSynapses + 1);
        brick.randValuePos = (brick.randValuePos + 1) % 1024;

        const float currentSideWeight = brick.randWeight[brick.randWeightPos] * weight;
        brick.randWeightPos = (brick.randWeightPos + 1) % 999;
        assert(currentSideWeight >= 0.0f);

        if(brick.globalValues.globalLearningOffset < 0.01f) {
            return;
        }

        // create new synapse if necessary
        if(choosePosition == currentSection.numberOfSynapses) {
            createNewSynapse(brick, currentSection);
        }

        // synapses, which are fully memorized, are not allowed to be overwritten!!!
        if(currentSection.synapses[choosePosition].memorize >= 0.99f) {
            continue;
        }

        // calculate new value
        const float newVal = brick.globalValues.globalLearningOffset * currentSideWeight;

        currentSection.synapses[choosePosition].weight += newVal;
        currentSection.totalWeight += abs(newVal);
    }
}

//==================================================================================================

/**
* process of a specific edge-section of a brick
*
* @param edgeSectionId id of the edge-section within the current brick
* @param weight incoming weight-value
*/
inline void
processSynapseSection(NetworkSegment &segment,
                      Brick &brick,
                      const uint32_t synapseSectionId,
                      const float inputWeight)
{
    SynapseSection* synapseSection = &getSynapseSectionBlock(segment)[synapseSectionId];
    if(synapseSection->status != ACTIVE_SECTION) {
        return;
    }

    learningSynapseSection(brick,
                           *synapseSection,
                           inputWeight - synapseSection->totalWeight);

    // limit ration to 1.0f
    float ratio = inputWeight / synapseSection->totalWeight;
    if(ratio > 1.0f) {
        ratio = 1.0f;
    }

    Synapse* end = synapseSection->synapses + synapseSection->numberOfSynapses;
    Node* nodes = &getNodeBlock(segment)[brick.nodePos];

    for(Synapse* synapse = synapseSection->synapses;
        synapse < end;
        synapse++)
    {
        const Synapse tempSynapse = *synapse;
        nodes[tempSynapse.targetNodeId].currentState +=
                tempSynapse.weight
                * ratio
                * ((float)tempSynapse.somaDistance / (float)MAX_SOMA_DISTANCE);
        synapse->inProcess = nodes[tempSynapse.targetNodeId].active;
    }
}

//==================================================================================================

} // namespace KyoukoMind

#endif // SYNAPSE_CONTAINER_PROCESSING_H
