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
class IncomingMessageBuffer;
class MessageBuffer;
class OutgoingMessageBuffer;

class DemoIO : public CommonThread
{
public:
    DemoIO(MessageController *messageController);

    void run();

    void sendOutData(const char input);

private:
    MessageController* m_messageController = nullptr;
    IncomingMessageBuffer* m_incomBuffer= nullptr;
    OutgoingMessageBuffer* m_ougoingBuffer= nullptr;

    void sendInnerData(const char input);

    void sendData(const KyoChanDirectEdgeContainer &edge);
    void sendFinishCycle();
};

}

#endif // DEMOIO_H
