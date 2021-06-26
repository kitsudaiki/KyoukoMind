#ifndef CORE_IO_H
#define CORE_IO_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

#include <libKitsunemimiAiCommon/metadata.h>

/**
 * @brief processInputNodes
 * @param segment
 * @param inputNodes
 */
inline void
processInputNodes(Segment* segment)
{
    for(uint64_t inputNodeId = 0;
        inputNodeId < segment->segmentHeader->inputs.count;
        inputNodeId++)
    {
        const InputNode* inputNode = &segment->inputs[inputNodeId];
        segment->nodes[inputNode->targetNode].input = inputNode->weight;
    }
}

/**
 * @brief processOutputNodes
 * @param segment
 * @param outputNodes
 */
inline void
processOutputNodes(Segment* segment)
{
    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        OutputNode* out = &segment->outputs[outputNodeId];
        Node* targetNode = &segment->nodes[out->targetNode];
        const float nodeWeight = targetNode->potential;
        out->outputWeight = 1.0f / (1.0f + exp(-1.0f * nodeWeight));
    }
}

/**
 * @brief calcTotalError
 * @param outputNodes
 * @param segmentHeader
 * @return
 */
inline float
calcTotalError(Segment* segment)
{
    float totalError = 0.0f;

    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        OutputNode* out = &segment->outputs[outputNodeId];
        const float diff = (out->shouldValue - out->outputWeight);
        totalError += 0.5f * (diff * diff);
    }

    //std::cout<<"error: "<<totalError<<std::endl;

    return totalError;
}

#endif // CORE_IO_H
