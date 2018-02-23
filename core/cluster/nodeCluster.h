#ifndef NODECLUSTER_H
#define NODECLUSTER_H

#include "cluster.h"

namespace KyoChan_Network
{

class NodeCluster : public Cluster
{
    Q_OBJECT

public:
    NodeCluster(quint32 clusterId,
                quint32 numberOfNodes,
                QString directoryPath = "",
                Cluster* parentCluster = nullptr);
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
};

}

#endif // NODECLUSTER_H
