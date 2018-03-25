#ifndef EDGECLUSTER_H
#define EDGECLUSTER_H

#include "emptyCluster.h"

namespace KyoukoMind
{

class EdgeCluster : public EmptyCluster
{

public:
    EdgeCluster(ClusterID clusterId,
                const std::string directoryPath);
};

}

#endif // EDGECLUSTER_H
