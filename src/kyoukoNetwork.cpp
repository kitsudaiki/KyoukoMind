/**
 *  @file    kyoukoNetwork.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <kyoukoNetwork.h>
#include <core/networkManager.h>

namespace KyoukoMind
{

// init static variables
KyoukoMind::Config* KyoukoNetwork::m_config = nullptr;
Kitsune::Chan::Communication::MindClient* KyoukoNetwork::m_mindClient = nullptr;
KyoukoMind::BrickHandler* KyoukoNetwork::m_brickHandler = nullptr;
KyoukoMind::GlobalValuesHandler* KyoukoNetwork::m_globalValuesHandler = nullptr;
KyoukoMind::MessageBlockBuffer* KyoukoNetwork::m_messageBuffer = nullptr;

/**
 * main-class
 */
KyoukoNetwork::KyoukoNetwork()
{
    m_config = new Config();
    m_mindClient = new Kitsune::Chan::Communication::MindClient();
    m_mindClient->initConnection();
    m_brickHandler = new BrickHandler();
    m_globalValuesHandler = new GlobalValuesHandler();

    m_messageBuffer = new MessageBlockBuffer();
}

/**
 * init all components
 */
void
KyoukoNetwork::start()
{
    // network-manager
    m_networkManager = new NetworkManager();
    m_networkManager->start();
}

}
