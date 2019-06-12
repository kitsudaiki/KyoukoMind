/**
 *  @file    mindClient.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MINDCLIENT_H
#define MINDCLIENT_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <communicationStructs/monitorinContianer.h>

namespace Kitsune
{
namespace Network
{
class TcpServer;
class TcpClient;
class NetworkTrigger;
}

namespace Chan
{
namespace Communication
{
struct MonitoringMetaData;
struct MonitoringProcessData;

class MindClient
{
public:
    MindClient();

    bool initConnection();

    bool addNetworkTrigger(Kitsune::Network::NetworkTrigger* trigger);

    bool sendData(uint8_t *message, const uint32_t size);

private:
    Kitsune::Network::TcpClient* m_client = nullptr;
};

}
}
}

#endif // MINDCLIENT_H
