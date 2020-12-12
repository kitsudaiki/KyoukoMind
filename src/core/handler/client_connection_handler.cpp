/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "client_connection_handler.h"

#include <kyouko_root.h>
#include <core/processing/objects/segment.h>

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
        Brick* brick = getBuffer<Brick>(KyoukoRoot::m_segment->bricks);
        const std::vector<float> output = brick[m_outputBrick].getOutputValues();
        for(uint32_t i = 0; i < output.size(); i++) {
            textOutput += static_cast<char>(output.at(i));
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

/**
 * @brief ClientConnectionHandler::registerInput
 * @param brickId
 * @return
 */
uint32_t
ClientConnectionHandler::registerInput(const uint32_t brickId)
{
    m_inputBrick = brickId;
    Brick* brick = getBuffer<Brick>(KyoukoRoot::m_segment->bricks);
    return brick[brickId].registerInput();
}

/**
 * @brief ClientConnectionHandler::registerOutput
 * @param brickId
 * @return
 */
uint32_t
ClientConnectionHandler::registerOutput(const uint32_t brickId)
{
    m_outputBrick = brickId;
    Brick* brick = getBuffer<Brick>(KyoukoRoot::m_segment->bricks);
    return brick[brickId].registerOutput();
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
    Brick* brick = getBuffer<Brick>(KyoukoRoot::m_segment->bricks);
    for(uint32_t i = 0; i < array->size(); i++) {
        brick[m_inputBrick].setInputValue(i, array->get(i)->getFloat());
    }

    return true;
}

