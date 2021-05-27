/**
 * @file        blossom_initializing.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef BLOSSOM_INITIALIZING_H
#define BLOSSOM_INITIALIZING_H

#include <common.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiPersistence/files/file_methods.h>
#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiPersistence/logger/logger.h>

#include <src/blossoms/special/special_blossoms.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

/**
 * @brief init special blossoms
 */
void
initSpecialBlossoms()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();

    assert(interface->addBlossom("special", "print", new PrintBlossom()));
    assert(interface->addBlossom("special", "assert", new AssertBlossom()));
    assert(interface->addBlossom("special", "item_update", new ItemUpdateBlossom()));
}

void
initBlossoms()
{
    initSpecialBlossoms();
}

#endif // BLOSSOM_INITIALIZING_H
