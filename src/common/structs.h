#ifndef STRUCTS_H
#define STRUCTS_H

#include <QObject>

enum ClusterType
{
    EMPTYCLUSTER = 0,
    EDGECLUSTER = 1,
    NODECLUSTER = 2
};

struct ClusterID
{
    quint32 x = 0;
    quint32 y = 0;
    quint32 z = 0;

    bool operator<(const ClusterID& rhs) const
    {
        if (x < rhs.x) {
           return true;
        }
        else if (x == rhs.x) {
            if (y < rhs.y) {
                return true;
            }
            else if (y == rhs.y) {
                return z < rhs.z;
            }
        }
        return false;
    }
} __attribute__((packed));

struct neighbor
{
    ClusterID clusterId;
    quint32 targetId = 0;
    ClusterType neighborType = EDGECLUSTER;
} __attribute__((packed));

struct CluserMetaData
{
    ClusterID clusterId;
    ClusterType clusterType = EDGECLUSTER;
    neighbor neighors[9];
    quint32 numberOfNodes = 0;
    quint32 positionNodeBlock = 0;
    quint32 numberOfNodeBlocks = 0;
    quint32 positionOfEdgeBlock = 0;
    quint32 numberOfEdgeBlocks = 0;

} __attribute__((packed));

struct KyoChanEdge
{
    float weight = 0.0;
    quint32 targetClusterId = 0;
    quint16 targetNodeId = 0;
} __attribute__((packed));

struct KyoChanEdgeSection
{
    quint16 axonIdInCluster = 0;
    quint16 numberOfActiveEdges = 0;
    KyoChanEdge edges[50];
    quint64 padding = 0;
} __attribute__((packed));

struct KyoChanNode
{
    float currentState = 0;
    qint32 border = 0;

    float nodePosInCluster[3];

    // Axon
    quint32 targetClusterId = 0;
    quint16 targetAxonId = 0;
} __attribute__((packed));

struct KyoChanAxon
{
    float currentState = 0;
    float nodePosInCluster[3];
} __attribute__((packed));


#endif // STRUCTS_H
