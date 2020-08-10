/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <kyouko_root.h>
#include <core/network_manager.h>
#include <core/object_handling/global_values.h>
#include <core/object_handling/segment.h>
#include <core/object_handling/brick.h>
#include <core/obj_converter.h>
#include <core/validation.h>
#include <core/events/event_processing.h>
#include <core/processing/gpu/gpu_processing_uint.h>

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
KyoukoMind::Segment* KyoukoRoot::m_segment = nullptr;
KyoukoMind::EventProcessing* KyoukoRoot::m_eventProcessing = nullptr;;

Kitsunemimi::Project::Session* KyoukoRoot::m_clientSession = nullptr;
Kitsunemimi::Project::Session* KyoukoRoot::m_controlSession = nullptr;
Kitsunemimi::Project::Session* KyoukoRoot::m_monitoringSession = nullptr;
std::map<uint32_t, Brick*>* KyoukoRoot::m_inputBricks = nullptr;

/**
 * main-class
 */
KyoukoRoot::KyoukoRoot()
{
    validateStructSizes();

    m_segment = new Segment();
    m_eventProcessing = new EventProcessing();
    m_eventProcessing->startThread();
    m_inputBricks = new std::map<uint32_t, Brick*>();

    m_sessionController = new Kitsunemimi::Project::SessionController(this, &sessionCallback,
                                                                      this, &clientCallback,
                                                                      this, &controlCallback,
                                                                      this, &errorCallback);
}

KyoukoRoot::~KyoukoRoot()
{
    m_sessionController->closeServer(m_serverId);
}

/**
 * init all components
 */
bool
KyoukoRoot::start()
{
    // network-manager
    m_networkManager = new NetworkManager();
    m_networkManager->startThread();

    return true;
    //return initServer();
}

/**
 * @brief RootObject::initServer
 * @return
 */
bool
KyoukoRoot::initServer()
{
    bool success = false;
    uint16_t port = static_cast<uint16_t>(GET_INT_CONFIG("Network", "port", success));

    LOG_INFO("create server on port " + std::to_string(port));
    m_serverId = m_sessionController->addTcpServer(port);
    return m_serverId != 0;
}

} // namespace KyoukoMind
