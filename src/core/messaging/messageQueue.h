#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <QObject>

#include <networkConnection.h>

class MessageQueue
{
public:
    MessageQueue();

private:
    NetworkConnection::NetworkConnection *m_networkConnection = nullptr;
};

#endif // MESSAGEQUEUE_H
