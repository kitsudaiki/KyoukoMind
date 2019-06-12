/**
 *  @file    kyoChanNetwork.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <common.h>

#include <settings/config.h>
#include <core/networkInteraction/mindClient.h>

#include <core/bricks/brickHandler.h>
#include <core/messaging/messageBlockBuffer.h>
#include <core/bricks/globalValuesHandler.h>

namespace KyoukoMind
{
class NetworkManager;
class BrickHandler;

class KyoukoNetwork
{

public:
    KyoukoNetwork();
    NetworkManager* getNodeManager() const;

    static KyoukoMind::Config* m_config;
    static Kitsune::Chan::Communication::MindClient* m_mindClient;
    static KyoukoMind::BrickHandler* m_brickHandler;
    static KyoukoMind::GlobalValuesHandler* m_globalValuesHandler;
    static KyoukoMind::MessageBlockBuffer* m_internalMessageBuffer;

    void start();

private:
    NetworkManager* m_networkManager = nullptr;

    bool initLogger(bool *ok);
};

}

#endif //NETWORK_H
