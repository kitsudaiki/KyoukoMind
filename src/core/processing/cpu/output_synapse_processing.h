#ifndef OUTPUT_SYNAPSE_PROCESSING_H
#define OUTPUT_SYNAPSE_PROCESSING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/item_buffer.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

inline void
outputSynapseProcessing(const uint32_t sectionPos,
                        float weight)
{
    float* outputNodes = getBuffer<float>(KyoukoRoot::m_segment->nodeOutputBuffer);
    OutputSynapseSection* synapseSections = &getBuffer<OutputSynapseSection>(KyoukoRoot::m_segment->outputSynapses)[sectionPos];

    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < 255
          && weight > 0.0f)
    {
        OutputSynapse* synapse = &synapseSections->synapses[pos];

        float ratio = weight / synapse->weightIn;
        ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

        outputNodes[synapse->targetNodeId] += ratio
                                              * synapse->weightOut
                                              * static_cast<float>(synapse->sign);
        weight -= synapse->weightIn;
        pos++;
    }

}


#endif // OUTPUT_SYNAPSE_PROCESSING_H
