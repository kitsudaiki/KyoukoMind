/**
 *  @file    kyoChanNetwork.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef KYSANETWORK_H
#define KYSANETWORK_H

#include <common.h>
#include <settings/config.h>
#include <monitoring/statusReporter.h>

namespace KyoukoMind
{
class NetworkManager;
class ClusterHandler;

class KyoukoNetwork
{

public:
    KyoukoNetwork();

    ClusterHandler* getClusterHandler() const;
    NetworkManager* getNodeManager() const;

    static KyoukoMind::Config* m_config;
    static KyoukoMind::StatusReporter* m_reporter;

private:
    NetworkManager* m_networkManager = nullptr;

    bool initLogger(bool *ok);
};

}

#endif //KYSANETWORK_H
