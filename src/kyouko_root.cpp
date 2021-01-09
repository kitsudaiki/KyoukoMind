/**
 * @file        kyouko_root.cpp
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

#include <kyouko_root.h>

#include <core/network_manager.h>
#include <core/objects/segment.h>
#include <core/objects/global_values.h>
#include <core/validation.h>
#include <core/connection_handler/client_connection_handler.h>
#include <core/connection_handler/monitoring_connection_handler.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>
#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

// init static variables
KyoukoRoot* KyoukoRoot::m_root = nullptr;
Segment* KyoukoRoot::m_segment = nullptr;
bool KyoukoRoot::m_freezeState = false;
ClientConnectionHandler* KyoukoRoot::m_clientHandler = nullptr;
MonitoringConnectionHandler* KyoukoRoot::m_monitoringHandler = nullptr;

MonitoringBrickMessage KyoukoRoot::monitoringBrickMessage;
MonitoringProcessingTimes KyoukoRoot::monitoringMetaMessage;

/**
 * @brief KyoukoRoot::KyoukoRoot
 */
KyoukoRoot::KyoukoRoot()
{
    validateStructSizes();

    m_root = this;
    m_freezeState = false;
    m_segment = new Segment();
    m_clientHandler = new ClientConnectionHandler();
    m_monitoringHandler = new MonitoringConnectionHandler();
}

/**
 * @brief KyoukoRoot::initializeSakuraFiles
 * @return
 */
bool
KyoukoRoot::initializeSakuraFiles()
{
    bool success = false;
    const std::string sakuraDir = GET_STRING_CONFIG("DEFAULT", "sakura-file-locaion", success);
    if(success == false) {
        return false;
    }

    std::string errorMessage = "";
    success = SakuraLangInterface::getInstance()->readFilesInDir(sakuraDir, errorMessage);
    if(success == false)
    {
        LOG_ERROR(errorMessage);
        return false;
    }

    return true;
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
 * @brief KyoukoRoot::learn
 * @param input
 * @param should
 * @param errorMessage
 * @return
 */
bool
KyoukoRoot::learn(const std::string &input,
                  const std::string &should,
                  std::string &)
{
    LOG_WARNING("input: " + input);
    LOG_WARNING("should: " + should);

    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Brick* inputBrick = KyoukoRoot::m_segment->inputBricks[0];
    Brick* outputBrick = KyoukoRoot::m_segment->outputBricks[0];

    const char* inputChar = input.c_str();
    for(uint32_t i = 0; i < input.size(); i++)
    {
        //const float value = (static_cast<float>(inputChar[i]) - 90.0f) * 10.0f;
        if(inputChar[i] == 'a') {
            inputBrick->setInputValue(i, globalValues->actionPotential);
        } else {
            inputBrick->setInputValue(i, 0.0f);
        }
    }

    const char* shouldChar = should.c_str();
    for(uint32_t i = 0; i < should.size(); i++)
    {
        const float value = (static_cast<float>(shouldChar[i]) - 90.0f) * 10.0f;
        outputBrick->setShouldValue(i, value);
    }

    return true;
}


