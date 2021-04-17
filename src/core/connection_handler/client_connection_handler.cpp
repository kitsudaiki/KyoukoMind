/**
 * @file        client_connection_handler.cpp
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

#include "client_connection_handler.h"

#include <kyouko_root.h>
#include <core/objects/segment.h>

#include <libKitsunemimiSakuraMessaging/messaging_client.h>
#include <libKitsunemimiJson/json_item.h>
#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief ClientHandler::ClientHandler
 */
ClientConnectionHandler::ClientConnectionHandler() {}

ClientConnectionHandler::~ClientConnectionHandler() {}

//==================================================================================================

/**
 * @brief send text to client
 *
 * @param text text to send
 *
 * @return false, if no gateway is connected, or send to gateway failed, else true
 */
bool
ClientConnectionHandler::sendToClient(const std::string &text)
{
    bool result = false;
    while(m_clientSession_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    if(m_client != nullptr) {
        result = m_client->sendStreamData(text.c_str(), text.size());
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
    JsonItem jsonItem;
    std::string errorMessage = "";
    if(jsonItem.parse(inputData, errorMessage) == false)
    {
        LOG_ERROR(errorMessage);
        return false;
    }

    DataArray* array = jsonItem.getItemContent()->toArray();

    return true;
}

