/**
 *  @file    emptyCluster.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef EMPTYCLUSTER_H
#define EMPTYCLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{
class MessageController;

class EmptyCluster : public Cluster
{

public:
    EmptyCluster(const ClusterID clusterId,
                 const std::string directoryPath,
                 MessageController *controller);

    EmptyCluster(const ClusterID clusterId,
                 const uint8_t clusterType,
                 const std::string directoryPath,
                 MessageController *controller);

    void finishCycle();
};

}

#endif // EMPTYCLUSTER_H
