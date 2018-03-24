#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <common.h>

class Message;
class DataMessage;
class ReplyMessage;
class LearningMessage;
class LearningReployMessage;

namespace KyoukoMind
{

class MessageQueue
{
public:
    MessageQueue();

    bool addMessageToQueue(Message* message);


private:
};

}

#endif // MESSAGEQUEUE_H
