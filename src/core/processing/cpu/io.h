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
 * @param nodes
 * @param inputNodes
 * @param segmentHeader
 */
inline void
processInputNodes(Node* nodes,
                  InputNode* inputNodes,
                  SegmentHeader* segmentHeader)
{
    for(uint64_t inputNodeId = 0;
        inputNodeId < segmentHeader->inputs.count;
        inputNodeId++)
    {
        nodes[inputNodes[inputNodeId].targetNode].input = inputNodes[inputNodeId].weight;
    }
}

/**
 * @brief processOutputNodes
 * @param nodes
 * @param outputNodes
 * @param segmentHeader
 */
inline void
processOutputNodes(Node* nodes,
                   OutputNode* outputNodes,
                   SegmentHeader* segmentHeader)
{
    for(uint64_t outputNodeId = 0;
        outputNodeId < segmentHeader->outputs.count;
        outputNodeId++)
    {
        OutputNode* out = &outputNodes[outputNodeId];
        Node* targetNode = &nodes[out->targetNode];
        float nodeWeight = targetNode->potential;
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
calcTotalError(OutputNode* outputNodes,
               SegmentHeader* segmentHeader)
{
    float totalError = 0.0f;

    for(uint64_t outputNodeId = 0;
        outputNodeId < segmentHeader->outputs.count;
        outputNodeId++)
    {
        OutputNode* out = &outputNodes[outputNodeId];
        const float diff = (out->shouldValue - out->outputWeight);
        totalError += 0.5f * (diff * diff);
    }

    return totalError;
}

#endif // CORE_IO_H
