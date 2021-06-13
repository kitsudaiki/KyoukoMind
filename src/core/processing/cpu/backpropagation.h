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
 * @param nodes
 * @param synapseSections
 * @param startPoint
 * @param numberOfNodes
 */
inline void
backpropagateNodes(Brick* brick,
                   Node* nodes,
                   SynapseSection* synapseSections)
{
    const uint32_t upperPos = brick->numberOfNodes + brick->nodePos;
    for(uint32_t nodeId = brick->nodePos; nodeId < upperPos; nodeId++)
    {
        Node* sourceNode = &nodes[nodeId];
        SynapseSection* section = &synapseSections[nodeId];

        if(section->active == 0) {
            continue;
        }

        uint16_t pos = 0;
        float netH = sourceNode->potential;
        const float outH = 1.0f / (1.0f + exp(-1.0f * netH));

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            Synapse* synapse = &section->synapses[pos];
            pos++;

            // process synapse
            if(synapse->targetNodeId != UNINIT_STATE_16)
            {
                netH -= static_cast<float>(synapse->border);

                // update weight
                const float delta = nodes[synapse->targetNodeId].delta;
                const float learnValue =  0.15f;
                const float diff = learnValue * delta * outH;
                Node* targetNode = &nodes[synapse->targetNodeId];
                sourceNode->delta += targetNode->delta * synapse->weight;

                synapse->weight -= diff;
            }
        }
    }
}

/**
 * @brief correctNewOutputSynapses
 * @param nodes
 * @param synapseSections
 * @param startPoint
 * @param numberOfNodes
 */
inline void
correctNewOutputSynapses(Brick* brick,
                         Node* nodes,
                         SynapseSection* synapseSections)
{
    const uint32_t upperPos = brick->numberOfNodes + brick->nodePos;
    for(uint32_t nodeId = brick->nodePos; nodeId < upperPos; nodeId++)
    {
        Node* sourceNode = &nodes[nodeId];
        SynapseSection* section = &synapseSections[nodeId];

        if(section->active == 0) {
            continue;
        }

        uint16_t pos = section->hardening;
        float netH = sourceNode->potential;

        // iterate over all synapses in the section and update the target-nodes
        while(pos < SYNAPSES_PER_SYNAPSESECTION
              && netH > 0.0f)
        {
            Synapse* synapse = &section->synapses[pos];
            pos++;

            // process synapse
            if(synapse->targetNodeId != UNINIT_STATE_16)
            {
                netH -= static_cast<float>(synapse->border);

                // update weight
                const float delta = nodes[synapse->targetNodeId].delta;
                const bool invert = (delta < 0.0f && synapse->weight < 0.0f)
                                    || (delta > 0.0f && synapse->weight > 0.0f);
                if(invert) {
                    synapse->weight *= -1.0f;
                }
            }
        }
    }
}

/**
 * @brief updateCoreSynapses
 * @param segmentMeta
 * @param synapseSections
 * @param nodes
 * @param synapseMetaData
 * @param threadId
 * @param numberOfThreads
 */
inline void
backpropagateOutput(CoreSegmentMeta* segmentMeta,
                    Node* nodes,
                    OutputNode* outputNodes)
{
    for(uint64_t i = 0; i < segmentMeta->numberOfOutputs; i++)
    {
        OutputNode* out = &outputNodes[i];
        Node* targetNode = &nodes[out->targetNode];
        const float outW = out->outputWeight;
        const float delta = (outW - out->shouldValue) * outW * (1.0f - outW);
        targetNode->delta = delta;
    }
}

#endif // CORE_BACKPROPAGATION_H
