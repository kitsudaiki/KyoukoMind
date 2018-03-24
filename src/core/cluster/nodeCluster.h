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
                const uint32_t numberOfNodes);
    NodeCluster(const ClusterID clusterId, const std::string directoryPath);
    ~NodeCluster();

    uint32_t getNumberOfEdgeBlocks();
    KyoChanEdgeSection *getEdgeBlock();
    void syncEdgeSections(uint32_t startSection = 0,
                          uint32_t endSection = 0);
    uint32_t getNumberOfNodeBlocks();
    KyoChanNode* getNodeBlock();

private:
    // node-stuff
    KyoChanNode* m_nodes = nullptr;

};

}

#endif // NODECLUSTER_H
