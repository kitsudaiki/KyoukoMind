/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <kyouko_root.h>

#include <core/network_manager.h>
#include <core/processing/objects/segment.h>
#include <core/validation.h>
#include <core/handler/client_handler.h>

#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>
#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <src/blossoms/register_input_blossom.h>
#include <src/blossoms/register_output_blossom.h>
#include <src/blossoms/learn_blossom.h>
#include <src/blossoms/metadata_blossom.h>
#include <src/blossoms/print_blossom.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

// init static variables
KyoukoRoot* KyoukoRoot::m_root = nullptr;
Segment* KyoukoRoot::m_segment = nullptr;
ClientHandler* KyoukoRoot::m_clientHandler = nullptr;

Kitsunemimi::Kyouko::MonitoringBrickMessage KyoukoRoot::monitoringBrickMessage;
Kitsunemimi::Kyouko::MonitoringProcessingTimes KyoukoRoot::monitoringMetaMessage;


/**
 * @brief KyoukoRoot::KyoukoRoot
 */
KyoukoRoot::KyoukoRoot()
{
    validateStructSizes();

    // test
    RegisterInputBlossom* newBlossom = new RegisterInputBlossom();
    SakuraLangInterface::getInstance()->addBlossom("test1",  "test2", newBlossom);

    m_root = this;
    m_segment = new Segment();
    m_clientHandler = new ClientHandler();
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
 * @brief KyoukoRoot::initBlossoms
 */
void
KyoukoRoot::initBlossoms()
{
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "register_input",
                                                          new RegisterInputBlossom()));
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "register_output",
                                                          new RegisterOutputBlossom()));
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "get_metadata",
                                                          new MetadataBlossom()));
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "learn",
                                                          new LearnBlossom()));
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "print",
                                                          new PrintBlossom()));
}

/**
 * @brief KyoukoRoot::initSakuraFiles
 * @return
 */
bool
KyoukoRoot::initSakuraFiles()
{
    bool success = false;
    const std::string sakuraDir = GET_STRING_CONFIG("DEFAULT", "sakura-file-locaion", success);
    if(success == false) {
        return false;
    }

    std::vector<std::string> sakuraFiles;
    if(Kitsunemimi::Persistence::listFiles(sakuraFiles, sakuraDir) == false)
    {
        LOG_ERROR("path with sakura-files doesn't exist: " + sakuraDir);
        return false;
    }

    std::string errorMessage = "";
    for(const std::string &filePath : sakuraFiles)
    {
        std::string content = "";
        if(Kitsunemimi::Persistence::readFile(content, filePath, errorMessage) == false)
        {
            LOG_ERROR("reading sakura-files failed with error: " + errorMessage);
            return false;
        }

        if(SakuraLangInterface::getInstance()->addTree("", content, errorMessage) == false)
        {
            LOG_ERROR("parsing sakura-files failed with error: " + errorMessage);
            return false;
        }
    }

    return true;
}

/**
 * @brief KyoukoRoot::learn
 * @param input
 * @param should
 * @param errorMessage
 * @return
 */
bool
KyoukoRoot::learn(const std::string &input,
                  const std::string &should,
                  std::string &errorMessage)
{
    return true;
}


