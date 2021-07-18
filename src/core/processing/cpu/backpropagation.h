#ifndef CORE_BACKPROPAGATION_H
#define CORE_BACKPROPAGATION_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

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
    uint16_t pos = 0;
    Node* sourceNode = nullptr;
    SynapseSection* section = nullptr;
    Synapse* synapse = nullptr;
    float netH = 0.0f;
    float outH = 0.0f;
    float learnValue = 0.0f;

    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        section = &segment->synapseSections[nodeId];
        if(section->active == 0) {
            continue;
        }

        pos = 0;
        sourceNode = &segment->nodes[nodeId];
        netH = sourceNode->potential;
        outH = 1.0f / (1.0f + exp(-1.0f * netH));

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            synapse = &section->synapses[pos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }

            // update weight
            learnValue = static_cast<float>(pos <= section->hardening) * 0.1f
                         + static_cast<float>(pos > section->hardening) * 0.2f;
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
    float outW = 0.0f;
    OutputNode* out = nullptr;
    Node* targetNode = nullptr;

    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        out = &segment->outputs[outputNodeId];
        targetNode = &segment->nodes[out->targetNode];
        outW = out->outputWeight;
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
    uint16_t pos = 0;
    Node* sourceNode = nullptr;
    SynapseSection* section = nullptr;
    Synapse* synapse = nullptr;
    float netH = 0.0f;
    float delta = 0.0f;
    float invert = 0.0f;

    for(uint32_t nodeId = brick->nodePos;
        nodeId < brick->numberOfNodes + brick->nodePos;
        nodeId++)
    {
        section = &segment->synapseSections[nodeId];
        if(section->active == 0) {
            continue;
        }

        pos = section->hardening;
        sourceNode = &segment->nodes[nodeId];
        netH = sourceNode->potential;

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            synapse = &section->synapses[pos];
            if(synapse->targetNodeId == UNINIT_STATE_16) {
                break;
            }

            // update weight
            delta = segment->nodes[synapse->targetNodeId].delta;
            invert = (delta < 0.0f && synapse->weight < 0.0f)
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
