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

inline void
backpropagateNodes(Node* nodes,
                   SynapseSection* synapseSections,
                   const uint32_t startPoint,
                   const uint32_t numberOfNodes)
{
    for(uint32_t i = startPoint; i < numberOfNodes + startPoint; i++)
    {
        Node* sourceNode = &nodes[i];
        if(sourceNode->border >= 0.0f)
        {
            SynapseSection* section = &synapseSections[i];
            if(section->active == 0) {
                continue;
            }

            uint16_t pos = 0;
            float weight = sourceNode->potential;
            // iterate over all synapses in the section and update the target-nodes
            while(pos < SYNAPSES_PER_SYNAPSESECTION
                  && weight > 0.0f)
            {
                Synapse* synapse = &section->synapses[pos];
                pos++;

                // process synapse
                if(synapse->targetNodeId != UNINIT_STATE_16)
                {
                    weight -= static_cast<float>(synapse->border);
                    const float val = static_cast<float>(SYNAPSES_PER_SYNAPSESECTION - pos);
                    const float netH = weight / val;
                    const float outH = 1.0f / (1.0f + exp(-1.0f * netH));

                    // update weight
                    const float delta = nodes[synapse->targetNodeId].delta;
                    const float learnValue =  0.5f;
                    synapse->weight -= learnValue * delta * outH;

                    Node* targetNode = &nodes[synapse->targetNodeId];
                    sourceNode->delta += targetNode->delta * synapse->weight * outH * (1.0f -  outH) * netH;
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
        targetNode->delta = (outW - out->shouldValue) * outW * (1.0f - outW);
        //std::cout<<"-----------"<<i<<" out: "<<out->outputWeight<<"  delta: "<<targetNode->delta<<std::endl;
    }
}

#endif // CORE_BACKPROPAGATION_H
