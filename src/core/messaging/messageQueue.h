#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <QObject>

#include <common/structs.h>
#include <networkConnection.h>

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

    bool addMessageToQueue(KyoChanMessage message);


private:
    NetworkConnection::NetworkConnection *m_networkConnection = nullptr;
};

}

#endif // MESSAGEQUEUE_H
