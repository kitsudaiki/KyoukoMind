/**
 *  @file    statusReporter.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef STATUSREPORTER_H
#define STATUSREPORTER_H

#include <common.h>
#include <structs/container.h>

namespace Kitsune
{
namespace Network
{
class TcpServer;
class TcpClient;
}
}

namespace KyoukoMind
{

class StatusReporter
{
public:
    StatusReporter();

    bool initServer(const uint16_t port);

    void sendStatus(const uint32_t clusterId,
                    const uint32_t x,
                    const uint32_t y,
                    const uint8_t type,
                    const MonitoringMetaData &metaData,
                    const MonitoringProcessData &processingData);
private:
    TcpClient* m_client = nullptr;
    //TcpServer* m_server = nullptr;
};

}

#endif // STATUSREPORTER_H
