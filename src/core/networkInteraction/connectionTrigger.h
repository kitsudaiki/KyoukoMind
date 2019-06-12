/**
 *  @file    connectionTrigger.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CONNECTIONTRIGGER_H
#define CONNECTIONTRIGGER_H

#include <networkTrigger.h>
#include <vector>
#include <string>
#include <common.h>
#include <communicationStructs/communicationMessages.h>

namespace Kitsune
{
namespace Network
{
class TcpClient;
struct MessageRingBuffer;
}
}
using Kitsune::Network::MessageRingBuffer;
using Kitsune::Chan::Communication::MessageSizes;

namespace KyoukoMind
{
class BrickHandler;

class ConnectionTrigger : public Kitsune::Network::NetworkTrigger
{
public:
    ConnectionTrigger();
    ~ConnectionTrigger();

    uint32_t runTask(const MessageRingBuffer &recvBuffer,
                     Kitsune::Network::TcpClient *client);

    uint8_t m_tempBuffer[8192];
    MessageSizes m_messageSize;

};

}

#endif // CONNECTIONTRIGGER_H
