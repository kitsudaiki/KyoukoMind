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
    ~NodeCluster();

    uint32_t getNumberOfNodeBlocks() const;
    uint16_t getNumberOfNodes() const;

    void addOutputCluster(const ClusterID clusterId,
                          const uint32_t outputAxonId,
                          const uint16_t nodeId);

    KyoChanNode* getNodeBlock();

    uint32_t getNumberOfEdgeBlocks() const;
    KyoChanEdgeSection* getEdgeBlock();
    bool initEdgeBlocks(const uint32_t numberOfEdgeSections);
    bool addEdge(const uint32_t edgeSectionId, const KyoChanEdge &newEdge);

    bool initNodeBlocks(uint16_t numberOfNodes);
    uint32_t addEmptyEdgeSection();

    uint16_t m_numberOfActiveNodes = 0;
};

}

#endif // NODECLUSTER_H
