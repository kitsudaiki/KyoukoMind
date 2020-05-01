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
#include <core/obj_converter.h>

#include <io/network_callbacks.h>
#include <io/client_processing.h>
#include <io/control_processing.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>

namespace KyoukoMind
{

// init static variables
KyoukoMind::BrickHandler* RootObject::m_brickHandler = nullptr;
KyoukoMind::GlobalValuesHandler* RootObject::m_globalValuesHandler = nullptr;
Kitsunemimi::Project::Session* RootObject::m_clientSession = nullptr;
Kitsunemimi::Project::Session* RootObject::m_controlSession = nullptr;
Kitsunemimi::Project::Session* RootObject::m_monitoringSession = nullptr;
std::map<uint32_t, Brick*>* RootObject::m_inputBricks = nullptr;

/**
 * main-class
 */
RootObject::RootObject()
{
    m_brickHandler = new BrickHandler();
    m_globalValuesHandler = new GlobalValuesHandler();
    m_inputBricks = new std::map<uint32_t, Brick*>();

    m_sessionController = new Kitsunemimi::Project::SessionController(this, &sessionCallback,
                                                                      this, &clientCallback,
                                                                      this, &controlCallback,
                                                                      this, &errorCallback);
}

RootObject::~RootObject()
{
    m_sessionController->closeServer(m_serverId);
}

/**
 * init all components
 */
bool
RootObject::start()
{
    // network-manager
    m_networkManager = new NetworkManager();
    m_networkManager->startThread();

    return initServer();
}

/**
 * @brief RootObject::initServer
 * @return
 */
bool
RootObject::initServer()
{
    bool success = false;
    uint16_t port = static_cast<uint16_t>(GET_INT_CONFIG("Network", "port", success));

    LOG_INFO("create server on port " + std::to_string(port));
    m_serverId = m_sessionController->addTcpServer(port);
    return m_serverId != 0;
}

/**
 * @brief RootObject::convertToObj
 * @param brickId
 * @param nodeId
 * @return
 */
const std::string
RootObject::convertToObj()
{
    m_networkManager->initBlockThread();
    // wait the double time of one cycle to ensure, that it is paused
    usleep(20000);

    std::string convertedString = "";
    convertNodeToString(convertedString, 18, 0);
    //convertNetworkToString(convertedString);

    std::string errorMessage = "";
    Kitsunemimi::Persistence::writeFile("/tmp/test_output.obj",
                                        convertedString,
                                        errorMessage,
                                        true);

    usleep(20000);

    m_networkManager->continueThread();

    return convertedString;
}

} // namespace KyoukoMind
