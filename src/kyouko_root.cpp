/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <kyouko_root.h>

#include <core/network_manager.h>
#include <core/object_handling/segment.h>
#include <core/validation.h>

#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>
#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <src/blossoms/test_blossom.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

// init static variables
Segment* KyoukoRoot::m_segment = nullptr;
std::map<uint32_t, Brick*>* KyoukoRoot::m_inputBricks = nullptr;

/**
 * @brief KyoukoRoot::KyoukoRoot
 */
KyoukoRoot::KyoukoRoot()
{
    validateStructSizes();

    // test
    TestBlossom* newBlossom = new TestBlossom();
    SakuraLangInterface::getInstance()->addBlossom("test1",  "test2", newBlossom);

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
