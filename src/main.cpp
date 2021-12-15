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
#include <callbacks.h>

#include <api/blossom_initializing.h>

#include <libKitsunemimiArgs/arg_parser.h>
#include <libKitsunemimiCommon/logger.h>

#include <libKitsunemimiHanamiCommon/generic_main.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>
#include <libKitsunemimiHanamiPredefinitions/init_predefined_blossoms.h>

using Kitsunemimi::Hanami::HanamiMessaging;
using Kitsunemimi::Hanami::initMain;

int
main(int argc, char *argv[])
{
    Kitsunemimi::ErrorContainer error;
    KyoukoRoot rootObj;

    if(initMain(argc, argv, "kyouko", &registerArguments, &registerConfigs, error) == false) {
        return 1;
    }

    bool success = false;
    const bool devMode = GET_BOOL_CONFIG("DevMode", "enable", success);
    if(devMode)
    {
        // run the dev-test based on the MNIST test files, if defined by the config
        const std::string initialFile = GET_STRING_CONFIG("DevMode", "file", success);
        const std::string configFile = GET_STRING_CONFIG("DevMode", "config", success);
        const std::string uuid = rootObj.initCluster(initialFile, error);

        const std::string mnistTestPath = GET_STRING_CONFIG("DevMode", "mnist_path", success);
        learnTestData(mnistTestPath, uuid);
    }
    else
    {
        // init blossoms
        initBlossoms();
        rootObj.init();
        if(rootObj.initializeSakuraFiles(error) == false)
        {
            LOG_ERROR(error);
            return 1;
        }

        Kitsunemimi::Hanami::initPredefinedBlossoms();

        // initialize server and connections based on the config-file
        const std::vector<std::string> groupNames = {};
        if(HanamiMessaging::getInstance()->initialize("kyouko",
                                                      groupNames,
                                                      nullptr,
                                                      streamDataCallback,
                                                      error,
                                                      true) == false)
        {
            LOG_ERROR(error);
            return 1;
        }
    }

    // sleep forever
    std::this_thread::sleep_until(std::chrono::time_point<std::chrono::system_clock>::max());

    return 0;
}
