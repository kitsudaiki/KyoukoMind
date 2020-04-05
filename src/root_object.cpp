/**
 *  @file    kyouko_network.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <root_object.h>
#include <core/network_manager.h>
#include <core/global_values_handler.h>

#include <libKitsunemimiPersistence/logger/logger.h>

namespace KyoukoMind
{

// init static variables
KyoukoMind::Config* RootObject::m_config = nullptr;
KyoukoMind::BrickHandler* RootObject::m_brickHandler = nullptr;
KyoukoMind::GlobalValuesHandler* RootObject::m_globalValuesHandler = nullptr;

/**
 * main-class
 */
RootObject::RootObject()
{
    m_config = new Config();
    m_brickHandler = new BrickHandler();
    m_globalValuesHandler = new GlobalValuesHandler();
}

/**
 * init all components
 */
void
RootObject::start()
{
    // network-manager
    m_networkManager = new NetworkManager();
}

} // namespace KyoukoMind
