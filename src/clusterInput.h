/**
 *  @file    clusterInput.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CLUSTERINPUT_H
#define CLUSTERINPUT_H

#include <common.h>

namespace KyoukoMind
{
class Cluster;
class ClusterHandler;
class NodeCluster;
class IncomingMessageBuffer;
class OutgoingMessageBuffer;
class GlobalValuesHandler;
}

class ClusterInput
{
public:
    ClusterInput(KyoukoMind::ClusterHandler *clusterHandler);

    void init(const uint32_t id, const uint32_t connectId);
    void sendData(const float input);

private:
    KyoukoMind::ClusterHandler* m_clusterHandler = nullptr;
    KyoukoMind::NodeCluster* m_fakeCluster = nullptr;
    KyoukoMind::OutgoingMessageBuffer* m_ougoingBuffer = nullptr;

    float edgeWeights[NUMBER_OF_NODES_PER_CLUSTER];
};

#endif // CLUSTERINPUT_H
