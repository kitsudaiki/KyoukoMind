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

#include <libKitsunemimiHanamiEndpoints/endpoint.h>

#include <api/v1/io/ask_blossom.h>
#include <api/v1/io/init_blossom.h>
#include <api/v1/io/learn_blossom.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

void
initTokenBlossoms()
{
    Kitsunemimi::Hanami::Endpoint* endpoints = Kitsunemimi::Hanami::Endpoint::getInstance();
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "io";

    assert(interface->addBlossom(group, "ask", new AskBlossom()));
    assert(endpoints->addEndpoint("v1/io",
                                  Kitsunemimi::Hanami::GET_TYPE,
                                  Kitsunemimi::Hanami::BLOSSOM_TYPE,
                                  group,
                                  "ask"));

    assert(interface->addBlossom(group, "learn", new LearnBlossom()));
    assert(endpoints->addEndpoint("v1/io",
                                  Kitsunemimi::Hanami::POST_TYPE,
                                  Kitsunemimi::Hanami::BLOSSOM_TYPE,
                                  group,
                                  "learn"));

    assert(interface->addBlossom(group, "init", new InitBlossom()));
    assert(endpoints->addEndpoint("v1/cluster",
                                  Kitsunemimi::Hanami::POST_TYPE,
                                  Kitsunemimi::Hanami::BLOSSOM_TYPE,
                                  group,
                                  "init"));
}

void
initBlossoms()
{
    initTokenBlossoms();
}

#endif // KYOUKOMIND_BLOSSOM_INITIALIZING_H
