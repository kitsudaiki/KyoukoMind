#ifndef EDGECLUSTER_H
#define EDGECLUSTER_H

#include "cluster.h"

namespace KyoChan_Network
{

class EdgeCluster : public Cluster
{
    Q_OBJECT

public:
    EdgeCluster(quint32 clusterId,
                Cluster* parentCluster = nullptr);

};

}

#endif // EDGECLUSTER_H
