/**
 * @file        monitoring_connection_handler.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
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
