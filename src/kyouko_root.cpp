/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <kyouko_root.h>
#include <core/network_manager.h>
#include <core/global_values.h>
#include <core/object_handling/segment.h>
#include <core/object_handling/brick.h>
#include <core/obj_converter.h>
#include <core/validation.h>
#include <core/processing/internal/gpu_interface.h>

#include <dummy_input.h>

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
KyoukoMind::GpuInterface* KyoukoRoot::m_gpuInterface = nullptr;

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
    m_inputBricks = new std::map<uint32_t, Brick*>();
    m_gpuInterface = new GpuInterface();

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

    // m_dummyInput = new DummyInput();
    // m_dummyInput->startThread();
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

/**
 * @brief RootObject::convertToObj
 * @param brickId
 * @param nodeId
 * @return
 */
const std::string
KyoukoRoot::convertToObj()
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
