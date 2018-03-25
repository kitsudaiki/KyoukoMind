#ifndef EMPTYCLUSTER_H
#define EMPTYCLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{
class IncomingMessageQueue;
class OutgoingMessageQueue;

class EmptyCluster : public Cluster
{

public:
    EmptyCluster(ClusterID clusterId,
                 const std::string directoryPath);

protected:
    IncomingMessageQueue* m_incomingMessageQueue = nullptr;
    OutgoingMessageQueue* m_outgoingMessageQueue = nullptr;
};

}

#endif // EMPTYCLUSTER_H
