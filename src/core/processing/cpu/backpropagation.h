#ifndef CORE_BACKPROPAGATION_H
#define CORE_BACKPROPAGATION_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

#include <libKitsunemimiAiCommon/metadata.h>

/**
 * @brief backpropagateNodes
 * @param brick
 * @param nodes
 * @param synapseSections
 */
inline void
backpropagateNodes(Brick* brick,
                   Segment* segment)
{
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        SynapseSection* section = &segment->synapseSections[nodeId];
        if(section->active == 0) {
            continue;
        }

        uint16_t pos = 0;
        Node* sourceNode = &segment->nodes[nodeId];
        float netH = sourceNode->potential;
        const float outH = 1.0f / (1.0f + exp(-1.0f * netH));

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            Synapse* synapse = &section->synapses[pos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }

            // update weight
            const float learnValue = static_cast<float>(pos <= section->hardening) * 0.1f
                                     + static_cast<float>(pos > section->hardening) * 0.4f;
            sourceNode->delta += segment->nodes[synapse->targetNodeId].delta * synapse->weight;
            synapse->weight -= learnValue * segment->nodes[synapse->targetNodeId].delta * outH;

            netH -= static_cast<float>(synapse->border) * BORDER_STEP;
            pos++;
        }
    }
}

/**
 * @brief backpropagateOutput
 * @param segmentHeader
 * @param nodes
 * @param outputNodes
 */
inline void
backpropagateOutput(Segment* segment)
{
    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        OutputNode* out = &segment->outputs[outputNodeId];
        Node* targetNode = &segment->nodes[out->targetNode];
        const float outW = out->outputWeight;
        targetNode->delta = (outW - out->shouldValue) * outW * (1.0f - outW);
    }
}

/**
 * @brief correctNewOutputSynapses
 * @param brick
 * @param nodes
 * @param synapseSections
 */
inline void
correctNewOutputSynapses(Brick* brick,
                         Segment* segment)
{
    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        SynapseSection* section = &segment->synapseSections[nodeId];
        if(section->active == 0) {
            continue;
        }

        uint16_t pos = section->hardening;
        Node* sourceNode = &segment->nodes[nodeId];
        float netH = sourceNode->potential;

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            Synapse* synapse = &section->synapses[pos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }

            // update weight
            const float delta = segment->nodes[synapse->targetNodeId].delta;
            const bool invert = (delta < 0.0f && synapse->weight < 0.0f)
                                || (delta > 0.0f && synapse->weight > 0.0f);
            if(invert) {
                synapse->weight *= -1.0f;
            }

            netH -= static_cast<float>(synapse->border) * BORDER_STEP;
            pos++;
        }
    }
}

#endif // CORE_BACKPROPAGATION_H
