#ifndef BLOSSOM_INITIALIZING_H
#define BLOSSOM_INITIALIZING_H

#include <common.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiPersistence/logger/logger.h>

#include <src/blossoms/register_input_blossom.h>
#include <src/blossoms/register_output_blossom.h>
#include <src/blossoms/learn_blossom.h>
#include <src/blossoms/metadata_blossom.h>
#include <src/blossoms/special_blossoms.h>
#include <src/blossoms/get_node_brick_ids_blossom.h>
#include <src/blossoms/set_global_values_blossom.h>
#include <src/blossoms/freeze_state_blossom.h>
#include <src/blossoms/snapshot_blossom.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

/**
 * @brief initBlossoms
 */
void
initBlossoms()
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
                                                          "get_node_brick_ids",
                                                          new GetNodeBrickIds_Blossom()));
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "set_global_values",
                                                          new SetGlobalValues_Blossom()));
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "freeze_state",
                                                          new FreezeStateBlossom()));
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "snapshot",
                                                          new SnapshotBlossom()));

    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "print",
                                                          new PrintBlossom()));
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "assert",
                                                          new AssertBlossom()));
    assert(SakuraLangInterface::getInstance()->addBlossom("special",
                                                          "item_update",
                                                          new ItemUpdateBlossom()));
}

/**
 * @brief initSakuraFiles
 * @return
 */
bool
initSakuraFiles()
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

#endif // BLOSSOM_INITIALIZING_H
