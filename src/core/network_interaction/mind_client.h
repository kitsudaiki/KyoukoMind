/**
 *  @file    mind_client.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MIND_CLIENT_H
#define MIND_CLIENT_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <communication_structs/monitorin_contianer.h>

namespace Kitsune
{
namespace Network
{
class TcpServer;
class TcpClient;
class NetworkTrigger;
} // namespace Network
} // namespace Kitsune

namespace KyoukoMind
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

} // namespace KyoukoMind

#endif // MIND_CLIENT_H
