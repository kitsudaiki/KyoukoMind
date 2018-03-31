#ifndef MESSAGEBUFFERTEST_H
#define MESSAGEBUFFERTEST_H

#include <common.h>

namespace KyoukoMind
{
class MessageController;
class IncomingMessageBuffer;
class MessageBuffer;
class OutgoingMessageBuffer;

class MessageBufferTest
{

public:
    MessageBufferTest();

private:
    void initTestCase();
    void checkMessageBuffer();
    void cleanupTestCase();

    MessageController* m_controller = nullptr;
    IncomingMessageBuffer* m_incomBuffer= nullptr;
    OutgoingMessageBuffer* m_ougoingBuffer= nullptr;
};

}

#endif // MESSAGEBUFFERTEST_H
