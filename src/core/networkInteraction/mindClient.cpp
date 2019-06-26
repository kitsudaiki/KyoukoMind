/**
 *  @file    mindClient.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/networkInteraction/mindClient.h>

#include <tcp/tcpClient.h>
#include <communicationStructs/mindContainer.h>
#include <communicationStructs/commonMessages.h>
#include <iostream>

namespace Kitsune
{
namespace Chan
{
namespace Communication
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
    SessionInit initMessage;
    initMessage.source = MIND;
    initMessage.target = TORII;

    return m_client->sendMessage((uint8_t*)(&initMessage), sizeof(SessionInit));
}

/**
 * add trigger-object to the client
 *
 * @return false, if client not exist, else result of the sub-call
 */
bool
MindClient::addNetworkTrigger(Network::NetworkTrigger *trigger)
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

}
}
}
