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
 * @param segmentMeta
 */
inline void
processInputNodes(Node* nodes,
                  InputNode* inputNodes,
                  CoreSegmentMeta* segmentMeta)
{
    for(uint64_t i = 0; i < segmentMeta->numberOfInputs; i++) {
        nodes[inputNodes[i].targetNode].input = inputNodes[i].weight;
    }
}

/**
 * @brief processOutputNodes
 * @param nodes
 * @param outputNodes
 * @param segmentMeta
 */
inline void
processOutputNodes(Node* nodes,
                   OutputNode* outputNodes,
                   CoreSegmentMeta* segmentMeta)
{
    for(uint64_t i = 0; i < segmentMeta->numberOfOutputs; i++)
    {
        OutputNode* out = &outputNodes[i];
        Node* targetNode = &nodes[out->targetNode];
        float nodeWeight = targetNode->potential;
        out->outputWeight = 1.0f / (1.0f + exp(-1.0f * nodeWeight));
    }
}

/**
 * @brief calcTotalError
 * @param outputNodes
 * @param segmentMeta
 * @return
 */
inline float
calcTotalError(OutputNode* outputNodes,
               CoreSegmentMeta* segmentMeta)
{
    float totalError = 0.0f;

    for(uint64_t i = 0; i < segmentMeta->numberOfOutputs; i++)
    {
        OutputNode* out = &outputNodes[i];
        const float diff = (out->shouldValue - out->outputWeight);
        totalError += 0.5f * (diff * diff);
    }

    return totalError;
}

#endif // CORE_IO_H
