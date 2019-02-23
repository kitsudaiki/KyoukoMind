/**
 *  @file    statusReporter.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <monitoring/statusReporter.h>

#include <tcp/tcpServer.h>
#include <tcp/tcpClient.h>

namespace KyoukoMind
{

/**
 * @brief StatusReporter::StatusReporter
 */
StatusReporter::StatusReporter()
{
    m_client = new TcpClient("127.0.0.1", 1337);
    //m_server = new TcpServer();
}

/**
 * @brief StatusReporter::initServer
 * @param port
 * @return
 */
bool StatusReporter::initServer(const uint16_t port)
{
    //if(m_server->initSocket(port))
    //{
     //   m_server->start();
        return true;
    //}
    //return false;
}

/**
 * @brief StatusReporter::sendStatus
 * @param clusterId
 * @param metaData
 * @param processingData
 */
void StatusReporter::sendStatus(const uint32_t clusterId,
                                const uint32_t x,
                                const uint32_t y,
                                const uint8_t type,
                                const MonitoringMetaData &metaData,
                                const MonitoringProcessData &processingData)
{
    MonitoringMessage message;
    message.clusterId = clusterId;
    message.xPos = x;
    message.yPos = y;
    message.type = type;
    message.metaData = metaData;
    message.processingData = processingData;

    m_client->sendMessage((uint8_t*)&message, sizeof(MonitoringMessage));
    //for(uint32_t i = 0; i < m_server->getNumberOfSockets(); i++)
    //{
    //    m_server->getSocket(i)->sendMessage((uint8_t*)&message, sizeof(MonitoringMessage));
    //}
}

}
