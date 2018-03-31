#ifndef EMPTYCLUSTER_H
#define EMPTYCLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{
class IncomingMessageBuffer;
class OutgoingMessageBuffer;

class EmptyCluster : public Cluster
{

public:
    EmptyCluster(ClusterID clusterId,
                 const std::string directoryPath,
                 KyoukoMind::MessageController *controller);

protected:
    IncomingMessageBuffer* m_incomingMessageQueue = nullptr;
    OutgoingMessageBuffer* m_outgoingMessageQueue = nullptr;
};

}

#endif // EMPTYCLUSTER_H
