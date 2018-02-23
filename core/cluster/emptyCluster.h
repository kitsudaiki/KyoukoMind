#ifndef EMPTYCLUSTER_H
#define EMPTYCLUSTER_H

#include "cluster.h"

namespace KyoChan_Network
{

class EmptyCluster : public Cluster
{
    Q_OBJECT

public:
    EmptyCluster(quint32 clusterId,
                 Cluster* parentCluster = nullptr);

};

}

#endif // EMPTYCLUSTER_H
