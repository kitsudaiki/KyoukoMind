/**
 * @file        main.cpp
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

#include <common.h>

#include <kyouko_root.h>
#include <args.h>
#include <config.h>
#include <core/callbacks.h>

#include <libKitsunemimiArgs/arg_parser.h>
#include <libKitsunemimiPersistence/logger/logger.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>

using Kitsunemimi::Sakura::MessagingController;

int
main(int argc, char *argv[])
{
    Kitsunemimi::Persistence::initConsoleLogger(true);

    // create and init argument-parser
    Kitsunemimi::Args::ArgParser argParser;
    registerArguments(argParser);

    // parse cli-input
    if(argParser.parse(argc, argv) == false) {
        return 1;
    }

    // init logging
    const bool enableDebug = argParser.wasSet("debug");
    Kitsunemimi::Persistence::initConsoleLogger(enableDebug);
    Kitsunemimi::Persistence::initFileLogger("/var/log/", "KyoukoMind", enableDebug);

    // init config
    const std::string configFile = argParser.getStringValue("config");
    if(Kitsunemimi::Config::initConfig(configFile) == false) {
        return 1;
    }
    registerConfigs();

    // create server
    KyoukoRoot* rootObject = new KyoukoRoot();
    rootObject->initBlossoms();
    if(rootObject->initSakuraFiles() == false) {
        return 1;
    }

    // initialize server and connections based on the config-file
    std::vector<std::string> groupNames = {};
    if(MessagingController::initializeMessagingController("KyoukoMind",
                                                          groupNames,
                                                          &sessionCreateCallback,
                                                          &sessionCloseCallback) == false)
    {
        return 1;
    }

    // start core
    rootObject->start();

    int a = 0;
    std::cin >> a;
    return 0;
}
