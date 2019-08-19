/**
 *  @file    connection_trigger.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CONNECTION_TRIGGER_H
#define CONNECTION_TRIGGER_H

#include <network_trigger.h>
#include <common.h>
#include <communication_structs/communication_messages.h>
#include <tcp/tcp_client.h>

using Kitsune::Network::MessageRingBuffer;
using Kitsune::Chan::Common::MessageSizes;

namespace KyoukoMind
{
class BrickHandler;

class ConnectionTrigger : public Kitsune::Network::NetworkTrigger
{
public:
    ConnectionTrigger();
    ~ConnectionTrigger();

    uint64_t runTask(const MessageRingBuffer &recvBuffer,
                     Kitsune::Network::TcpClient *client);

    uint8_t m_tempBuffer[8192];
    MessageSizes m_messageSize;

};

} // namespace KyoukoMind

#endif // CONNECTION_TRIGGER_H
