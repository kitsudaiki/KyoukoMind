/**
 *  @file    using.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef USING_H
#define USING_H

#include <structs/container.h>

using Kitsune::Chan::Monitoring::MonitoringMetaData;
using Kitsune::Chan::Monitoring::MonitoringProcessData;
using Kitsune::Chan::Monitoring::MonitoringMessage;


#include <tcp/tcpServer.h>
#include <tcp/tcpClient.h>

using Kitsune::Network::TcpClient;
using Kitsune::Network::TcpServer;

#endif // USING_H
