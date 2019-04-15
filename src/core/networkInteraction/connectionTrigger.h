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

namespace Kitsune
{
namespace Network
{
class TcpClient;
}
}

namespace KyoukoMind
{
class ClusterHandler;

class ConnectionTrigger : public Kitsune::Network::NetworkTrigger
{
public:
    ConnectionTrigger(ClusterHandler *model);
    ~ConnectionTrigger();

    void runTask(uint8_t* buffer,
                 const long bufferSize,
                 Kitsune::Network::TcpClient* client);

private:
    ClusterHandler* m_model = nullptr;

};

}

#endif // CONNECTIONTRIGGER_H
