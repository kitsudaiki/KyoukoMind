#include "messageQueue.h"

MessageQueue::MessageQueue()
{
    m_networkConnection = new NetworkConnection::NetworkConnection();
}

