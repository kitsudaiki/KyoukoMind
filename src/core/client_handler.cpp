/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "client_handler.h"

#include <libKitsunemimiSakuraMessaging/messaging_client.h>

/**
 * @brief ClientHandler::ClientHandler
 */
ClientHandler::ClientHandler() {}

ClientHandler::~ClientHandler() {}

/**
 * @brief ClientHandler::sendToClient
 * @param data
 * @param dataSize
 * @return
 */
bool
ClientHandler::sendToClient(const void *data,
                            const uint32_t dataSize)
{
    bool result = false;
    while(m_clientSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    if(m_client != nullptr) {
        result = m_client->sendStreamData(data, dataSize);;
    }

    m_clientSession_lock.clear(std::memory_order_release);
    return result;
}

/**
 * @brief ClientHandler::sendToMonitoring
 * @param data
 * @param dataSize
 * @return
 */
bool
ClientHandler::sendToMonitoring(const void *data,
                                const uint32_t dataSize)
{
    bool result = false;
    while(m_monitoringSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    if(m_client != nullptr) {
        result = m_monitoring->sendStreamData(data, dataSize);
    }

    m_monitoringSession_lock.clear(std::memory_order_release);
    return result;
}

/**
 * @brief set net client-session
 *
 * @return pointer to client-session
 */
void
ClientHandler::setClientSession(MessagingClient* session)
{
    while(m_clientSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_client = session;
    m_clientSession_lock.clear(std::memory_order_release);
}

/**
 * @brief set net monitoring-session
 *
 * @return pointer to monitoring-session
 */
void
ClientHandler::setMonitoringSession(MessagingClient* session)
{
    while(m_monitoringSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_monitoring = session;
    m_monitoringSession_lock.clear(std::memory_order_release);
}

/**
 * @brief get client session
 *
 * @return pointer to client-session
 */
MessagingClient*
ClientHandler::getClientSession()
{
    MessagingClient* session = nullptr;
    while(m_clientSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    session = m_client;
    m_clientSession_lock.clear(std::memory_order_release);
    return session;
}

/**
 * @brief get monitoring session
 *
 * @return pointer to monitoring-session
 */
MessagingClient*
ClientHandler::getMonitoringSession()
{
    MessagingClient* session = nullptr;
    while(m_monitoringSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    session = m_monitoring;
    m_monitoringSession_lock.clear(std::memory_order_release);
    return session;
}
