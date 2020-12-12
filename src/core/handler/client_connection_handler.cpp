/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "client_connection_handler.h"

#include <kyouko_root.h>
#include <core/processing/objects/segment.h>

#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief ClientHandler::ClientHandler
 */
ClientConnectionHandler::ClientConnectionHandler() {}

ClientConnectionHandler::~ClientConnectionHandler() {}

//==================================================================================================

/**
 * @brief ClientHandler::sendToClient
 * @return
 */
bool
ClientConnectionHandler::sendToClient()
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
ClientConnectionHandler::setClientSession(MessagingClient* session)
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
ClientConnectionHandler::getClientSession()
{
    MessagingClient* session = nullptr;
    while(m_clientSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    session = m_client;
    m_clientSession_lock.clear(std::memory_order_release);
    return session;
}

//==================================================================================================

/**
 * @brief ClientHandler::processInput
 * @param input
 * @return
 */
bool
ClientConnectionHandler::insertInput(const std::string &inputData)
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
ClientConnectionHandler::registerInput(const uint32_t pos, const uint32_t range)
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
ClientConnectionHandler::getInput(const uint32_t pos)
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
ClientConnectionHandler::registerOutput()
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
ClientConnectionHandler::setOutput(const uint32_t pos, const float value)
{
    while(m_output_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    m_outputs[pos] += value;
    m_outputs[pos] /= 2.0f;
    m_output_lock.clear(std::memory_order_release);
}

//==================================================================================================
