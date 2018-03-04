#ifndef STRUCTS_H
#define STRUCTS_H

#include <QObject>
#include <common/enums.h>

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
    bool operator=(const ClusterID& rhs) const
    {
        if (x == rhs.x && y == rhs.y && z == rhs.z) {
           return true;
        }
        return false;
    }
} __attribute__((packed));

struct Neighbor
{
    ClusterID targetClusterId;
    ClusterType neighborType = EMPTYCLUSTER;
    quint32 numberOfConnections = 0;
} __attribute__((packed));

struct ClusterMetaData
{
    ClusterID clusterId;
    ClusterType clusterType = EMPTYCLUSTER;
    Neighbor neighors[9];
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

struct KyoChanNewEdge
{
    float weight = 0.0;
    quint32 sourceClusterId = 0;
    quint16 sourceNodeId = 0;
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
    float border = 0;

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

struct KyoChanMessage
{

};


#endif // STRUCTS_H
