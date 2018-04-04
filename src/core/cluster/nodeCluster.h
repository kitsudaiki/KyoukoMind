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

    uint32_t getNumberOfNodeBlocks() const;
    uint32_t getNumberOfAxonBlocks() const;
    uint32_t getNumberOfEdgeBlocks() const;

    KyoChanNode* getNodeBlock();
    KyoChanAxon* getAxonBlock();
    KyoChanEdgeSection *getEdgeBlock();

    bool addEdge(const uint32_t axonId, const KyoChanEdge &newEdge);

    bool initNodeBlocks(uint16_t numberOfNodes);
    bool initAxonBlocks(uint32_t numberOfAxons);
    bool initEdgeBlocks(uint32_t numberOfEdgeSections);


    void syncEdgeSections(uint32_t startSection = 0,
                          uint32_t endSection = 0);

private:
    // node-stuff
    KyoChanNode* m_nodes = nullptr;

    bool isEdgeSectionFull(const uint32_t edgeSectionPos);
    bool addEmptyEdgeSection(const uint32_t axonId);
};

}

#endif // NODECLUSTER_H
