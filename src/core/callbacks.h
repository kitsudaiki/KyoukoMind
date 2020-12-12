/**
 * @file       callbacks.h
 *
 * @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright  Apache License Version 2.0
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

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <kyouko_root.h>
#include <core/connection_handler/client_connection_handler.h>
#include <core/connection_handler/monitoring_connection_handler.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>
#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief clientDataCallback
 * @param data
 * @param dataSize
 */
void
clientDataCallback(Kitsunemimi::Sakura::Session*,
                   const void* data,
                   const uint64_t dataSize)
{
    const char* charData = static_cast<const char*>(data);
    const std::string text(charData, dataSize);
    LOG_WARNING("client-text: " + text);
}

/**
 * @brief monitoringDataCallback
 */
void
monitoringDataCallback(Kitsunemimi::Sakura::Session*,
                       const void*,
                       const uint64_t)
{
}

/**
 * @brief sessionCallback
 * @param target
 * @param isInit
 * @param session
 * @param identifier
 */
void
sessionCreateCallback(Kitsunemimi::Sakura::MessagingClient* session,
                      const std::string identifier)
{
    if(identifier == "client")
    {
        session->setStreamMessageCallback(&clientDataCallback);
        KyoukoRoot::m_clientHandler->setClientSession(session);
    }
    if(identifier == "monitoring")
    {
        session->setStreamMessageCallback(&monitoringDataCallback);
        KyoukoRoot::m_monitoringHandler->setMonitoringSession(session);
    }
}

void
sessionCloseCallback(const std::string identifier)
{
    if(identifier == "client") {
        KyoukoRoot::m_clientHandler->setClientSession(nullptr);
    }
    if(identifier == "monitoring") {
        KyoukoRoot::m_monitoringHandler->setMonitoringSession(nullptr);
    }
}

#endif // CALLBACKS_H
