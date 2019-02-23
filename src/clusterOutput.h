/**
 *  @file    clusterOutput.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CLUSTEROUTPUT_H
#define CLUSTEROUTPUT_H

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

class ClusterOutput
{
public:
    ClusterOutput(KyoukoMind::ClusterHandler *clusterHandler);

    void init(const uint32_t id, const uint32_t connectId);
    std::string getOutput();
    void setLearningOverride(float value);

private:
    KyoukoMind::ClusterHandler* m_clusterHandler = nullptr;
    KyoukoMind::NodeCluster* m_fakeCluster = nullptr;
    KyoukoMind::IncomingMessageBuffer* m_incomBuffer = nullptr;
    KyoukoMind::NodeCluster* m_outgoingCluster = nullptr;

    float m_outBuffer[10];
    uint8_t m_outBufferPos = 0;

};

#endif // CLUSTEROUTPUT_H
