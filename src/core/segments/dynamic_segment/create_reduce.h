/**
 * @file        create_reduce.h
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

#ifndef KYOUKOMIND_CREATE_REDUCE_H
#define KYOUKOMIND_CREATE_REDUCE_H

#include <common.h>

#include <kyouko_root.h>
#include <core/segments/brick.h>

#include "objects.h"
#include "dynamic_segment.h"

/**
 * @brief reduce synapses of a specific section
 *
 * @param segment segment where the section belongs to
 * @param section section to resuce
 *
 * @return true, if section is empty and can be deleted, else false
 */
inline bool
reduceSynapses(DynamicSegment &segment,
               SynapseSection &section)
{
    Synapse* synapse = nullptr;
    Synapse currentSyn;
    uint32_t currentPos = 0;

    // iterate over all synapses in synapse-section
    currentPos = section.hardening;
    for(uint32_t lastPos = section.hardening;
        lastPos < SYNAPSES_PER_SYNAPSESECTION;
        lastPos++)
    {
        // skip not connected synapses
        synapse = &section.synapses[lastPos];
        if(synapse->targetNodeId == UNINIT_STATE_16) {
            continue;
        }

        // update dynamic-weight-value of the synapse
        if(segment.nodes[synapse->targetNodeId].active == 0) {
            synapse->activeCounter = -2;
        } else {
            synapse->activeCounter = -2;
        }

        // check for deletion of the single synapse
        if(synapse->activeCounter < 0)
        {
            synapse->weight = 0.0f;
            synapse->targetNodeId = UNINIT_STATE_16;
            synapse->border = 0;
        }
        else
        {
            currentSyn = section.synapses[currentPos];
            section.synapses[currentPos] = section.synapses[lastPos];
            section.synapses[lastPos] = currentSyn;
            currentPos++;
        }
    }

    if(section.next != UNINIT_STATE_32)
    {
        // delete if sections is empty
        const bool shouldDelete = reduceSynapses(segment, segment.synapseSections[section.next]);
        if(shouldDelete)
        {
            segment.segmentData.deleteItem(section.next);
            section.next = UNINIT_STATE_32;
        }
    }

    const bool shouldDelete = section.hardening == 0
                              && currentPos == 0
                              && section.next == UNINIT_STATE_32;
    return shouldDelete;
}

/**
 * @brief reduce all synapses within the segment and delete them, if the reach a deletion-border
 *
 * @param segment current segemnt to process
 */
inline void
reduceNodes(DynamicSegment &segment)
{
    SynapseSection* section = nullptr;
    DynamicNode* sourceNode = nullptr;
    uint32_t sectionId = 0;
    bool shouldDelete = false;

    for(uint32_t nodeId = 0;
        nodeId < segment.segmentHeader->nodes.count;
        nodeId++)
    {
        sourceNode = &segment.nodes[nodeId];
        if(sourceNode->targetSectionId == UNINIT_STATE_32) {
            continue;
        }

        // set start-values
        sectionId = sourceNode->targetSectionId;
        section = &segment.synapseSections[sectionId];

        // delete if sections is empty
        shouldDelete = reduceSynapses(segment, *section);
        if(shouldDelete)
        {
            segment.segmentData.deleteItem(sectionId);
            sourceNode->targetSectionId = UNINIT_STATE_32;
        }
    }
}

#endif // KYOUKOMIND_CREATE_REDUCE_H
