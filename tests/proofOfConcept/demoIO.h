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

namespace KyoukoMind
{
class MessageController;
class ClusterHandler;
class IncomingMessageBuffer;
class MessageBuffer;
class OutgoingMessageBuffer;

class DemoIO : public CommonThread
{
public:
    DemoIO(MessageController *messageController,
           ClusterHandler *clusterHandler);

    void run();

    void sendOutData(const char input);
    void sendFinishCycle();


private:
    MessageController* m_messageController = nullptr;
    ClusterHandler* m_clusterHandler = nullptr;
    IncomingMessageBuffer* m_incomBuffer= nullptr;
    OutgoingMessageBuffer* m_ougoingBuffer= nullptr;

    void sendInnerData(const char input);

    void sendData(const KyoChanDirectEdgeContainer &edge);
};

}

#endif // DEMOIO_H
