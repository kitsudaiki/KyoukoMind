/**
 *  @file    demoIO.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef DEMOIO_H
#define DEMOIO_H

#include <common.h>
#include <commonThread.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{
class ClusterHandler;
class NodeCluster;
class IncomingMessageBuffer;
class OutgoingMessageBuffer;
class GlobalValuesHandler;

class DemoIO : public Kitsune::CommonThread
{
public:
    DemoIO(ClusterHandler *clusterHandler);

    void run();

    void sendOutData(const char input);
    void sendPositive();
    void sendNegative();
    void resetLearning();

private:
    ClusterHandler* m_clusterHandler = nullptr;
    NodeCluster* m_fakeCluster = nullptr;
    IncomingMessageBuffer* m_incomBuffer = nullptr;
    OutgoingMessageBuffer* m_ougoingBuffer = nullptr;
    GlobalValuesHandler* m_globalValuesHandler = nullptr;

    char convertOutput(float one, float two, float three);
    void sendInnerData(const char input);
    void sendData(const KyoChanDirectEdgeContainer &edge);
};

}

#endif // DEMOIO_H
