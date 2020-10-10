/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <kyouko_root.h>

#include <core/network_manager.h>
#include <core/object_handling/segment.h>
#include <core/validation.h>

#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>
#include <libKitsunemimiSakuraMessaging/messaging_client.h>

// init static variables
Segment* KyoukoRoot::m_segment = nullptr;
std::map<uint32_t, Brick*>* KyoukoRoot::m_inputBricks = nullptr;

/**
 * @brief KyoukoRoot::KyoukoRoot
 */
KyoukoRoot::KyoukoRoot()
{
    validateStructSizes();

    std::string errorMessage = "";

    bool ret = Kitsunemimi::Persistence::writeFile("/tmp/KyoukoMind.conf",
                                        getTestConfig(),
                                        errorMessage,
                                        true);

    Kitsunemimi::Config::initConfig("/tmp/KyoukoMind.conf");
    std::vector<std::string> groupNames = {"ToriiGateway"};
    Kitsunemimi::Sakura::MessagingController::initializeMessagingController("KyoukoMind", groupNames);
    Kitsunemimi::Sakura::MessagingClient* client = Kitsunemimi::Sakura::MessagingController::getInstance()->getClient("ToriiGateway");


    m_segment = new Segment();
    m_inputBricks = new std::map<uint32_t, Brick*>();
}

/**
 * @brief KyoukoRoot::~KyoukoRoot
 */
KyoukoRoot::~KyoukoRoot() {}

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
 * @brief Session_Test::getTestConfig
 * @return
 */
const std::string
KyoukoRoot::getTestConfig()
{
    const std::string config = "[DEFAULT]\n"
                               "port = 12346\n"
                               "\n"
                               "\n"
                               "[ToriiGateway]\n"
                               "port = 12345\n"
                               "address = \"127.0.0.1\"\n";
    return config;
}
