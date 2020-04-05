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
#include <core/objects/brick.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>

namespace KyoukoMind
{

// init static variables
KyoukoMind::Config* RootObject::m_config = nullptr;
KyoukoMind::BrickHandler* RootObject::m_brickHandler = nullptr;
KyoukoMind::GlobalValuesHandler* RootObject::m_globalValuesHandler = nullptr;
Kitsunemimi::Project::Session* RootObject::m_clientSession = nullptr;
Kitsunemimi::Project::Session* RootObject::m_monitoringSession = nullptr;

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
bool
RootObject::start()
{
    // network-manager
    m_networkManager = new NetworkManager();

    return initServer();
}

bool
RootObject::initServer()
{
    bool success = false;
    uint16_t port = static_cast<uint16_t>(GET_INT_CONFIG("Kyouko", "port", success));

    LOG_INFO("create server on port " + std::to_string(port));
    m_serverId = m_sessionController->addTcpServer(port);
    return m_serverId != 0;
}

} // namespace KyoukoMind
