/**
 *  @file    clusterQueue.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CLUSTERQUEUE_H
#define CLUSTERQUEUE_H

#include <common.h>

namespace KyoukoMind
{
class Cluster;
class GlobalValuesHandler;

class ClusterQueue
{
public:
    ClusterQueue(GlobalValuesHandler* globalValuesHandler);
    ~ClusterQueue();

    bool clearQueue();
    Cluster* getCluster();
    bool addCluster(Cluster* cluster);

    GlobalValuesHandler *getGlobalValuesHandler() const;

private:
    std::queue<Cluster*> m_queue;
    GlobalValuesHandler* m_globalValuesHandler = nullptr;
};

}

#endif // CLUSTERQUEUE_H
