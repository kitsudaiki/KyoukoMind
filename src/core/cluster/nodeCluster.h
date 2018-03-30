#ifndef NODECLUSTER_H
#define NODECLUSTER_H

#include "edgeCluster.h"

namespace KyoukoMind
{

class NodeCluster : public EdgeCluster
{

public:
    NodeCluster(const ClusterID clusterId,
                const std::string directoryPath,
                const uint32_t numberOfNodes,
                MessageController *controller);
    NodeCluster(const ClusterID clusterId,
                const std::string directoryPath,
                MessageController *controller);
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
