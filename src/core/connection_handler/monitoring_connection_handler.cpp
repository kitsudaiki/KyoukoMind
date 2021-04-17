/**
 * @file        monitoring_connection_handler.cpp
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

#include "monitoring_connection_handler.h"

#include <kyouko_root.h>
#include <core/objects/segment.h>

#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief MonitoringConnectionHandler::MonitoringConnectionHandler
 */
MonitoringConnectionHandler::MonitoringConnectionHandler() {}

MonitoringConnectionHandler::~MonitoringConnectionHandler() {}

//==================================================================================================

/**
 * @brief MonitoringConnectionHandler::sendToMonitoring
 * @return
 */
bool
MonitoringConnectionHandler::sendToMonitoring()
{
    std::string monitoringOutput = "{\"bricks\": [";
    Brick* brick = KyoukoRoot::m_synapseSegment->nodeBricks;

    bool found = false;
    for(uint32_t i = 0; i < KyoukoRoot::m_synapseSegment->segmentMeta->numberOfNodeBricks; i++)
    {
        if(brick[i].brickId != UNINIT_STATE_32)
        {
            if(found) {
                monitoringOutput += ",";
            }
            const std::string part = "[" + std::to_string(brick[i].brickPos.x)
                                   + "," + std::to_string(brick[i].brickPos.y)
                                   + "," + std::to_string(brick[i].nodeActivity) + "]";

            brick[i].nodeActivity = 0;
            monitoringOutput += part;
            found = true;
        }
    }
    monitoringOutput += "]}";
    return sendToMonitoring(monitoringOutput.c_str(), monitoringOutput.size());
}

/**
 * @brief MonitoringConnectionHandler::sendToMonitoring
 * @param data
 * @param dataSize
 * @return
 */
bool
MonitoringConnectionHandler::sendToMonitoring(const char* data, const uint64_t dataSize)
{
    bool result = false;
    while(m_monitoringSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    if(m_monitoring != nullptr) {
        result = m_monitoring->sendStreamData(data, dataSize);
    }

    m_monitoringSession_lock.clear(std::memory_order_release);
    return result;
}

/**
 * @brief set net monitoring-session
 *
 * @return pointer to monitoring-session
 */
void
MonitoringConnectionHandler::setMonitoringSession(MessagingClient* session)
{
    while(m_monitoringSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_monitoring = session;
    m_monitoringSession_lock.clear(std::memory_order_release);
}

/**
 * @brief get monitoring session
 *
 * @return pointer to monitoring-session
 */
MessagingClient*
MonitoringConnectionHandler::getMonitoringSession()
{
    MessagingClient* session = nullptr;
    while(m_monitoringSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    session = m_monitoring;
    m_monitoringSession_lock.clear(std::memory_order_release);
    return session;
}
