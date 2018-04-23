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
class EdgeProcessing;

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
    AxonProcessing* m_axonProcessing = nullptr;
    NodeProcessing* m_nodeProcessing = nullptr;
    EdgeProcessing* m_edgeProcessing = nullptr;
};

}

#endif // PROCESSINGTEST_H
