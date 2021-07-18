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
    InputNode* inputNode = nullptr;

    for(uint64_t inputNodeId = 0;
        inputNodeId < segment->segmentHeader->inputs.count;
        inputNodeId++)
    {
        inputNode = &segment->inputs[inputNodeId];
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
    OutputNode* out = nullptr;
    Node* targetNode = nullptr;
    float nodeWeight = 0.0f;

    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        out = &segment->outputs[outputNodeId];
        targetNode = &segment->nodes[out->targetNode];
        nodeWeight = targetNode->potential;
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
    OutputNode* out = nullptr;
    float diff = 0.0f;

    for(uint64_t outputNodeId = 0;
        outputNodeId < segment->segmentHeader->outputs.count;
        outputNodeId++)
    {
        out = &segment->outputs[outputNodeId];
        diff = (out->shouldValue - out->outputWeight);
        totalError += 0.5f * (diff * diff);
    }

    //std::cout<<"error: "<<totalError<<std::endl;

    return totalError;
}

#endif // CORE_IO_H
