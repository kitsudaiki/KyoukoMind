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
class EdgeCluster;

class ClusterQueue
{
public:
    ClusterQueue();
    ~ClusterQueue();

    bool clearQueue();
    EdgeCluster* getCluster();
    bool addCluster(EdgeCluster* cluster);

private:
    std::queue<EdgeCluster*> m_queue;
};

}

#endif // CLUSTERQUEUE_H
