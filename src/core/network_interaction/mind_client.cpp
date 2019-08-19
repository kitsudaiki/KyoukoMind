/**
 *  @file    mind_client.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/network_interaction/mind_client.h>

#include <tcp/tcp_client.h>

#include <communication_structs/mind_container.h>
#include <communication_structs/common_messages.h>
#include <iostream>

namespace KyoukoMind
{

MindClient::MindClient()
{
}

/**
 * init the connection to the server
 *
 * @return result of the sub-call
 */
bool
MindClient::initConnection()
{
    std::cout<<"init mind connection"<<std::endl;
    m_client = new Kitsune::Network::TcpClient("127.0.0.1", 1337);

    usleep(1000);
    m_client->start();

    // init session
    Kitsune::Chan::Common::SessionInit initMessage;
    initMessage.source = MIND;
    initMessage.target = TORII;

    return m_client->sendMessage((uint8_t*)(&initMessage),
                                 sizeof(Kitsune::Chan::Common::SessionInit));
}

/**
 * add trigger-object to the client
 *
 * @return false, if client not exist, else result of the sub-call
 */
bool
MindClient::addNetworkTrigger(Kitsune::Network::NetworkTrigger* trigger)
{
    // precheck
    if(m_client == nullptr) {
        return false;
    }

    return m_client->addNetworkTrigger(trigger);
}

/**
 * send data to the server
 *
 * @return result of the sub-call
 */
bool
MindClient::sendData(uint8_t* message, const uint32_t size)
{
    return m_client->sendMessage(message, size);
}

} // namespace KyoukoMind
