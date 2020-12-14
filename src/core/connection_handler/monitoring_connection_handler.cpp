/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
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
    Brick* brick = getBuffer<Brick>(KyoukoRoot::m_segment->bricks);

    bool found = false;
    for(uint32_t i = 0; i < KyoukoRoot::m_segment->bricks.numberOfItems; i++)
    {
        if(brick[i].brickId != UNINIT_STATE_32)
        {
            if(found) {
                monitoringOutput += ",";
            }
            const std::string part = "[" + std::to_string(brick[i].brickPos.x)
                                   + "," + std::to_string(brick[i].brickPos.y)
                                   + "," + std::to_string(brick[i].nodeActivity)
                                   + "," + std::to_string(brick[i].synapseActivity) + "]";
            brick[i].nodeActivity = 0;
            brick[i].synapseActivity = 0;
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
