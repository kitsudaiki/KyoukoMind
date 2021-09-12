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
#include <dev_test.h>

#include <api/blossom_initializing.h>

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

    // init config by using the file defined over the CLI-input or the default config file
    std::string configFile = "/etc/KyoukoMind/KyoukoMind.conf";
    if(argParser.wasSet("config")) {
        configFile = argParser.getStringValue("config");
    }
    if(Kitsunemimi::Config::initConfig(configFile) == false) {
        return 1;
    }
    registerConfigs();

    // create core
    KyoukoRoot* rootObject = new KyoukoRoot();
    rootObject->start();

    bool success = false;
    const bool devMode = GET_BOOL_CONFIG("DevMode", "enable", success);
    if(devMode)
    {
        // run the dev-test based on the MNIST test files, if defined by the config
        const std::string initialFile = GET_STRING_CONFIG("DevMode", "file", success);
        const std::string configFile = GET_STRING_CONFIG("DevMode", "config", success);
        const std::string uuid = rootObject->initCluster(initialFile);

        const std::string mnistTestPath = GET_STRING_CONFIG("DevMode", "mnist_path", success);
        learnTestData(mnistTestPath, uuid);
    }
    else
    {
        // init blossoms
        initBlossoms();
        if(rootObject->initializeSakuraFiles() == false) {
            return 1;
        }

        // initialize server and connections based on the config-file
        std::vector<std::string> groupNames = {};
        const bool sakuraMessageInit = MessagingController::initializeMessagingController(
                    "KyoukoMind",
                    groupNames,
                    &sessionCreateCallback,
                    &sessionCloseCallback);
        if(sakuraMessageInit == false)
        {
            return 1;
        }
    }

    // sleep forever
    std::this_thread::sleep_until(std::chrono::time_point<std::chrono::system_clock>::max());

    return 0;
}
