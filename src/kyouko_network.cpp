/**
 *  @file    kyouko_network.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <kyouko_network.h>
#include <core/network_manager.h>
#include <core/bricks/global_values_handler.h>

namespace KyoukoMind
{

// init static variables
KyoukoMind::Config* KyoukoNetwork::m_config = nullptr;
KyoukoMind::BrickHandler* KyoukoNetwork::m_brickHandler = nullptr;
KyoukoMind::GlobalValuesHandler* KyoukoNetwork::m_globalValuesHandler = nullptr;
KyoukoMind::MessageBlockBuffer* KyoukoNetwork::m_messageBuffer = nullptr;

/**
 * main-class
 */
KyoukoNetwork::KyoukoNetwork()
{
    m_config = new Config();
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
}

} // namespace KyoukoMind
