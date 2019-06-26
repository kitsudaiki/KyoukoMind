/**
 *  @file    kyoukoNetwork.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef KYOUKONETWORK_H
#define KYOUKONETWORK_H

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

    static KyoukoMind::Config* m_config;
    static Kitsune::Chan::Communication::MindClient* m_mindClient;
    static KyoukoMind::BrickHandler* m_brickHandler;
    static KyoukoMind::GlobalValuesHandler* m_globalValuesHandler;
    static KyoukoMind::MessageBlockBuffer* m_messageBuffer;

    void start();

private:
    NetworkManager* m_networkManager = nullptr;
};

}

#endif //KYOUKONETWORK_H
