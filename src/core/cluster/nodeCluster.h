#ifndef NODECLUSTER_H
#define NODECLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{

class NodeCluster : public Cluster
{
    Q_OBJECT

public:
    NodeCluster(const ClusterID clusterId,
                const QString directoryPath,
                const quint32 numberOfNodes);
    NodeCluster(const ClusterID clusterId, const QString directoryPath);
    ~NodeCluster();

    quint32 getNumberOfEdgeBlocks();
    KyoChanEdgeSection *getEdgeBlock();
    void syncEdgeSections(quint32 startSection = 0,
                          quint32 endSection = 0);
    quint32 getNumberOfNodeBlocks();
    KyoChanNode* getNodeBlock();

private:
    // node-stuff
    KyoChanNode* m_nodes = nullptr;

    void initFile(const ClusterID clusterId,
                  const QString directoryPath);
};

}

#endif // NODECLUSTER_H
