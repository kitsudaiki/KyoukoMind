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

/**
 * @brief outputSynapseProcessing
 * @param sectionPos
 * @param weight
 * @return
 */
inline uint64_t
outputSynapseProcessing(const uint32_t sectionPos,
                        float weight)
{
    ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* synapseSections = &getBuffer<OutputSynapseSection>(*buf)[sectionPos];
    uint64_t activeOnes = 0;
    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < 255
          && weight > 0.0f)
    {
        OutputSynapse* synapse = &synapseSections->synapses[pos];

        if(synapse->targetNodeId == UNINIT_STATE_16)
        {
            synapse->hardening = 0.0f;

            // set new weight
            synapse->weightIn = 2.0f;
            synapse->weightOut = 2.0f;
            synapse->weightOut *= static_cast<float>(1 - (rand() % 2) * 2);
            synapse->targetNodeId = 1;
        }

        if(synapse->newOne == 1) {
            activeOnes++;
        }

        float ratio = weight / synapse->weightIn;
        ratio = (ratio > 1.0f) * 1.0f + (ratio <= 1.0f) * ratio;

        KyoukoRoot::m_segment->outputValue += ratio * synapse->weightOut;
        weight -= synapse->weightIn;
        pos++;
    }

    return activeOnes;
}

/**
 * @brief outputSynapseLearn
 * @param sectionPos
 * @param diff
 * @return
 */
inline uint64_t
outputSynapseLearn(const uint32_t sectionPos,
                   float diff,
                   float weight)
{
    ItemBuffer* buf = &KyoukoRoot::m_segment->outputSynapses;
    OutputSynapseSection* synapseSections = &getBuffer<OutputSynapseSection>(*buf)[sectionPos];
    uint64_t activeOnes = 0;
    uint32_t pos = 0;

    // iterate over all synapses in the section and update the target-nodes
    while(pos < 255
          && weight > 0.0f)
    {
        OutputSynapse* synapse = &synapseSections->synapses[pos];

        if(synapse->newOne == 1) {
            synapse->weightOut += diff;
        }

        weight -= synapse->weightIn;
        pos++;
    }

    return activeOnes;
}

/**
 * @brief node_processing
 */
bool
output_node_processing()
{
    GlobalValues* globalValue = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    float* outputNodes = getBuffer<float>(KyoukoRoot::m_segment->nodeOutputBuffer);
    uint64_t activeOnes = 0;

    for(uint32_t i = 0; i < globalValue->numberOfNodesPerBrick; i++) {
        activeOnes += outputSynapseProcessing(i, outputNodes[i]);
    }

    if(KyoukoRoot::m_segment->doLearn)
    {
        if(activeOnes == 0) {
            return false;
        }

        float diff = KyoukoRoot::m_segment->shouldValue - KyoukoRoot::m_segment->outputValue;
        diff /= static_cast<float>(activeOnes);

        std::cout<<"diff: "<<diff<<std::endl;
        for(uint32_t i = 0; i < globalValue->numberOfNodesPerBrick; i++) {
            activeOnes = outputSynapseLearn(i, diff, outputNodes[i]);
        }
    }

    return true;
}

#endif // OUTPUT_SYNAPSE_PROCESSING_H
