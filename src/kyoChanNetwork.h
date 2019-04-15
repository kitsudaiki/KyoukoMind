/**
 *  @file    kyoChanNetwork.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef KYOCHANNETWORK_H
#define KYOCHANNETWORK_H

#include <common.h>
#include <settings/config.h>
#include <mindClient.h>

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
    static Kitsune::Chan::Communication::MindClient* m_mindClient;

private:
    NetworkManager* m_networkManager = nullptr;

    bool initLogger(bool *ok);
};

}

#endif //KYOCHANNETWORK_H
