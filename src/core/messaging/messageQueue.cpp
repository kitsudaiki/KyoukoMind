#include "messageQueue.h"

namespace KyoukoMind
{

MessageQueue::MessageQueue()
{
    m_networkConnection = new NetworkConnection::NetworkConnection();
}

}
