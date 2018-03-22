#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H


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

class MessageController
{
public:
    MessageController();

private:
    NetworkConnection::NetworkConnection *m_networkConnection = nullptr;
};

}

#endif // MESSAGECONTROLLER_H
