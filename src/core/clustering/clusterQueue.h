/**
 *  @file    clusterQueue.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CLUSTERQUEUE_H
#define CLUSTERQUEUE_H

#include <common.h>

namespace KyoukoMind
{
class Cluster;

class ClusterQueue
{
public:
    ClusterQueue();
    ~ClusterQueue();

    bool clearQueue();
    Cluster* getCluster();
    bool addCluster(Cluster* cluster);

private:
    std::queue<Cluster*> m_queue;
};

}

#endif // CLUSTERQUEUE_H
