/**
 *  @file    nodeCluster.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef NODECLUSTER_H
#define NODECLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{

class NodeCluster : public Cluster
{

public:
    NodeCluster(const ClusterID clusterId,
                const std::string directoryPath,
                const uint32_t numberOfNodes = 0);

    // nodes
    uint32_t getNumberOfNodeBlocks() const;
    uint32_t getNumberOfNodes() const;
    KyoChanNode* getNodeBlock();
    bool initNodeBlocks(uint32_t numberOfNodes);

    // edge-sections
    uint32_t getNumberOfEdgeSectionBlocks() const;
    uint32_t getNumberOfEdgeSections() const;
    KyoChanEdgeSection* getEdgeSectionBlock();
    bool initEdgeSectionBlocks(const uint32_t numberOfEdgeSections);
    uint32_t addEmptyEdgeSection(const uint8_t sourceSide,
                                 const uint32_t sourceId);
    bool addEdge(const uint32_t edgeSectionId, const KyoChanEdge &newEdge);
};

}

#endif // NODECLUSTER_H
