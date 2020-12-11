/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "client_handler.h"

#include <kyouko_root.h>
#include <core/processing/objects/segment.h>

#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief ClientHandler::ClientHandler
 */
ClientHandler::ClientHandler() {}

ClientHandler::~ClientHandler() {}

//==================================================================================================

/**
 * @brief ClientHandler::sendToClient
 * @return
 */
bool
ClientHandler::sendToClient()
{
    bool result = false;
    while(m_clientSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    if(m_client != nullptr)
    {
        std::string textOutput = "";
        for(uint32_t i = 0; i < m_outputs.size(); i++) {
            textOutput += static_cast<char>(m_outputs.at(i));
        }
        result = m_client->sendStreamData(textOutput.c_str(), textOutput.size());
    }

    m_clientSession_lock.clear(std::memory_order_release);
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

//==================================================================================================

/**
 * @brief ClientHandler::sendToMonitoring
 * @return
 */
bool
ClientHandler::sendToMonitoring()
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
                                   + "," + std::to_string(brick[i].activity) + "]";
            brick[i].activity = 0;
            monitoringOutput += part;
            found = true;
        }
    }
    monitoringOutput += "]}";
    return sendToMonitoring(monitoringOutput.c_str(), monitoringOutput.size());
}

/**
 * @brief ClientHandler::sendToMonitoring
 * @param data
 * @param dataSize
 * @return
 */
bool
ClientHandler::sendToMonitoring(const char* data, const uint64_t dataSize)
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
ClientHandler::setMonitoringSession(MessagingClient* session)
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
ClientHandler::getMonitoringSession()
{
    MessagingClient* session = nullptr;
    while(m_monitoringSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    session = m_monitoring;
    m_monitoringSession_lock.clear(std::memory_order_release);
    return session;
}

//==================================================================================================

/**
 * @brief ClientHandler::processInput
 * @param input
 * @return
 */
bool
ClientHandler::insertInput(const std::string &inputData)
{
    bool result = false;
    while(m_input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    if(inputData.size() <= m_inputs.size())
    {
        for(uint32_t i = 0; i < inputData.size(); i++) {
            m_inputs[i].value = static_cast<float>(inputData.at(i));
        }
    }
    m_input_lock.clear(std::memory_order_release);
    return result;
}

/**
 * @brief ClientHandler::registerInput
 * @param pos
 * @param range
 * @return
 */
uint32_t
ClientHandler::registerInput(const uint32_t pos, const uint32_t range)
{
    uint32_t listPos = 0;
    while(m_input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_inputs.push_back(InputObj(pos, range));
    listPos = static_cast<uint32_t>(m_inputs.size()) - 1;
    m_input_lock.clear(std::memory_order_release);
    return listPos;
}

/**
 * @brief ClientHandler::getInput
 * @return
 */
InputObj
ClientHandler::getInput(const uint32_t pos)
{
    InputObj item;
    while(m_input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    item = m_inputs.at(pos);
    m_input_lock.clear(std::memory_order_release);
    return item;
}

//==================================================================================================

/**
 * @brief ClientHandler::registerOutput
 * @return
 */
uint32_t
ClientHandler::registerOutput()
{
    uint32_t pos = 0;
    while(m_output_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_outputs.push_back(0.0f);
    pos = static_cast<uint32_t>(m_outputs.size()) - 1;
    m_output_lock.clear(std::memory_order_release);
    return pos;
}

/**
 * @brief ClientHandler::setOutput
 * @param pos
 * @param value
 */
void
ClientHandler::setOutput(const uint32_t pos, const float value)
{
    while(m_output_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_outputs[pos] += value;
    m_outputs[pos] /= 2.0f;
    m_output_lock.clear(std::memory_order_release);
}

//==================================================================================================
