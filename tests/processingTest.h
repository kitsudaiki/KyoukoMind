#ifndef PROCESSINGTEST_H
#define PROCESSINGTEST_H

#include <common.h>
#include <tests/commonTest.h>

namespace KyoukoMind
{
class NodeCluster;
class MessageController;
class IncomingMessageBuffer;
class MessageBuffer;
class OutgoingMessageBuffer;

class NextChooser;
class AxonProcessing;
class NodeProcessing;
class ClusterProcessing;
class NodeClusterProcessing;

class ProcessingTest : public CommonTest
{
public:
    ProcessingTest();

private:
    void initTestCase();
    void checkProcessing();
    void cleanupTestCase();

    NodeCluster *m_nodeCluster1 = nullptr;
    NodeCluster *m_nodeCluster2 = nullptr;
    NodeCluster *m_nodeCluster3 = nullptr;

    MessageController* m_controller = nullptr;
    IncomingMessageBuffer* m_incomBuffer= nullptr;
    OutgoingMessageBuffer* m_ougoingBuffer= nullptr;

    NextChooser* m_nextChooser = nullptr;
    NodeClusterProcessing* m_nodeProcessing = nullptr;
};

}

#endif // PROCESSINGTEST_H
