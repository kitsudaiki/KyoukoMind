/**
 *  @file    demoIO.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef DEMOIO_H
#define DEMOIO_H

#include <common.h>
#include <core/processing/processingThreads/commonThread.h>
#include <core/structs/messageContainer.h>

namespace Networking
{
class IncomingMessageBuffer;
class OutgoingMessageBuffer;
}

namespace KyoukoMind
{
class ClusterHandler;
class NodeCluster;

class DemoIO : public CommonThread
{
public:
    DemoIO(ClusterHandler *clusterHandler);

    void run();

    void sendOutData(const char input);
    void sendFinishCycle();


private:
    ClusterHandler* m_clusterHandler = nullptr;
    NodeCluster* m_fakeCluster = nullptr;
    Networking::IncomingMessageBuffer* m_incomBuffer = nullptr;
    Networking::OutgoingMessageBuffer* m_ougoingBuffer = nullptr;

    void sendInnerData(const char input);

    void sendData(const KyoChanDirectEdgeContainer &edge);
};

}

#endif // DEMOIO_H
