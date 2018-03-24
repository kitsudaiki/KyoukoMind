#ifndef EMPTYCLUSTER_H
#define EMPTYCLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{

class EmptyCluster : public Cluster
{

public:
    EmptyCluster(ClusterID clusterId,
                 const std::string directoryPath);
};

}

#endif // EMPTYCLUSTER_H
