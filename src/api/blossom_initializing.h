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

#ifndef KYOUKOMIND_BLOSSOM_INITIALIZING_H
#define KYOUKOMIND_BLOSSOM_INITIALIZING_H

#include <common.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCommon/common_methods/file_methods.h>
#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiCommon/logger.h>

#include <api/special/special_blossoms.h>


#include <api/io/ask_blossom.h>
#include <api/io/init_blossom.h>
#include <api/io/learn_blossom.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

void
initTokenBlossoms()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "io";

    assert(interface->addBlossom(group, "ask", new AskBlossom()));
    assert(interface->addBlossom(group, "learn", new LearnBlossom()));
    assert(interface->addBlossom(group, "init", new InitBlossom()));
}

/**
 * @brief init special blossoms
 */
void
initSpecialBlossoms()
{
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "-";

    assert(interface->addBlossom(group, "print", new PrintBlossom()));
    assert(interface->addBlossom(group, "assert", new AssertBlossom()));
    assert(interface->addBlossom(group, "item_update", new ItemUpdateBlossom()));

}

void
initBlossoms()
{
    initSpecialBlossoms();
    initTokenBlossoms();
}

#endif // KYOUKOMIND_BLOSSOM_INITIALIZING_H
