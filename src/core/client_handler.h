/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <atomic>

namespace Kitsunemimi {
namespace Sakura {
class MessagingClient;
}
}

using Kitsunemimi::Sakura::MessagingClient;

class ClientHandler
{
public:
    ClientHandler();

    void setClientSession(MessagingClient* session);
    void setMonitoringSession(MessagingClient* session);

    MessagingClient* getClientSession();
    MessagingClient* getMonitoringSession();

private:
    std::atomic_flag m_clientSession_lock = ATOMIC_FLAG_INIT;
    MessagingClient* m_client = nullptr;

    std::atomic_flag m_monitoringSession_lock = ATOMIC_FLAG_INIT;
    MessagingClient* m_monitoring = nullptr;
};

#endif // CLIENT_HANDLER_H
