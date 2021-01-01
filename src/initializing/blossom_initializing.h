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
#include <src/blossoms/set_global_values_blossom.h>
#include <src/blossoms/freeze_state_blossom.h>
#include <src/blossoms/snapshot_blossom.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

/**
 * @brief initSpecialBlossoms
 */
void
initSpecialBlossoms()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();

    assert(interface->addBlossom("special", "print", new PrintBlossom()));
    assert(interface->addBlossom("special", "assert", new AssertBlossom()));
    assert(interface->addBlossom("special", "item_update", new ItemUpdateBlossom()));

    assert(interface->addBlossom("special", "learn", new LearnBlossom()));
    assert(interface->addBlossom("special", "freeze_state", new FreezeStateBlossom()));
}

/**
 * @brief initRegisterBlossoms
 */
void
initRegisterBlossoms()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();

    assert(interface->addBlossom("register", "input", new RegisterInputBlossom()));
    assert(interface->addBlossom("register", "output",  new RegisterOutputBlossom()));
}

/**
 * @brief initMetadataBlossoms
 */
void
initMetadataBlossoms()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();

    assert(interface->addBlossom("metadata", "get", new MetadataBlossom()));
}

/**
 * @brief initGlobalValuesBlossoms
 */
void
initGlobalValuesBlossoms()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();

    assert(interface->addBlossom("global_values", "set", new SetGlobalValues_Blossom()));
}

/**
 * @brief initSnapshotBlossoms
 */
void
initSnapshotBlossoms()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();

    assert(interface->addBlossom("snapshot", "obj", new SnapshotBlossom()));
}
/**
 * @brief initBlossoms
 */
void
initBlossoms()
{
    initSpecialBlossoms();
    initRegisterBlossoms();
    initMetadataBlossoms();
    initGlobalValuesBlossoms();
    initSnapshotBlossoms();
}

#endif // BLOSSOM_INITIALIZING_H
