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
class IncomingMessageBuffer;
class OutgoingMessageBuffer;

class EmptyCluster : public Cluster
{

public:
    EmptyCluster(const ClusterID clusterId,
                 const std::string directoryPath,
                 KyoukoMind::MessageController *controller);

    EmptyCluster(const ClusterID clusterId,
                 const uint8_t clusterType,
                 const std::string directoryPath,
                 MessageController *controller);

    void finishCycle();

protected:
    IncomingMessageBuffer* m_incomingMessageQueue = nullptr;
    OutgoingMessageBuffer* m_outgoingMessageQueue = nullptr;

private:
    void initMessageBuffer(const ClusterID clusterId,
                           MessageController *controller);
};

}

#endif // EMPTYCLUSTER_H
