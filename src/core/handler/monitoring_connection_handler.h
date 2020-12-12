/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef MONITORING_CONNECTION_HANDLER_H
#define MONITORING_CONNECTION_HANDLER_H

#include <common.h>

namespace Kitsunemimi {
namespace Sakura {
class MessagingClient;
}
}

using Kitsunemimi::Sakura::MessagingClient;

class MonitoringConnectionHandler
{
public:
    MonitoringConnectionHandler();
    ~MonitoringConnectionHandler();

    bool sendToMonitoring();
    bool sendToMonitoring(const char* data, const uint64_t dataSize);

    void setMonitoringSession(MessagingClient* session);
    MessagingClient* getMonitoringSession();

private:
    std::atomic_flag m_monitoringSession_lock = ATOMIC_FLAG_INIT;
    MessagingClient* m_monitoring = nullptr;
};

#endif // MONITORING_CONNECTION_HANDLER_H
