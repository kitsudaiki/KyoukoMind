#ifndef STRUCTS_H
#define STRUCTS_H

#include <QObject>

struct KyoChanMetaData
{
    quint8 toggleFlag = 0;
    quint32 clusterId = 0;
    quint8 clusterType = 0;
    quint32 neighors[7];
    quint32 numberOfNodes = 0;
    quint32 positionNodeBlock = 0;
    quint32 numberOfNodeBlocks = 0;
    quint32 positionOfEdgeBlock = 0;
    quint32 numberOfEdgeBlocks = 0;

} __attribute__((packed));

struct KyoChanEdge
{
    qint16 weight = 0;
    quint16 targetClusterId = 0;
    quint16 targetNodeId = 0;
} __attribute__((packed));

struct KyoChanEdgeSection
{
    quint16 nodeNumberInCluster = 0;
    KyoChanEdge edges[85];
} __attribute__((packed));

struct KyoChanNode
{
    qint32 currentState = 0;
    qint32 border = 1;
    bool outputNode = false;

    float nodePosInCluster[3];

    quint32 axonClusterId = 0;
    float axonPosInCLuster[3];
} __attribute__((packed));

#endif // STRUCTS_H
