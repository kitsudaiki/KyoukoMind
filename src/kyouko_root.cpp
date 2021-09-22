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

#include <core/validation.h>

#include <core/processing/cpu/cpu_processing_unit.h>
#include <core/processing/segment_queue.h>
#include <core/processing/processing_unit_handler.h>

#include <core/processing/network_cluster.h>
#include <core/objects/node.h>
#include <core/storage_io.h>
#include <core/cluster_handler.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/text_file.h>

#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>
#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

// init static variables
KyoukoRoot* KyoukoRoot::m_root = nullptr;
ClusterHandler* KyoukoRoot::m_clusterHandler = nullptr;
uint32_t* KyoukoRoot::m_randomValues = nullptr;
SegmentQueue* KyoukoRoot::m_segmentQueue = nullptr;
ProcessingUnitHandler* KyoukoRoot::m_processingUnitHandler = nullptr;

/**
 * @brief KyoukoRoot::KyoukoRoot
 */
KyoukoRoot::KyoukoRoot()
{
    validateStructSizes();

    m_root = this;

    // init predefinde random-values
    m_randomValues = new uint32_t[NUMBER_OF_RAND_VALUES];
    for(uint32_t i = 0; i < NUMBER_OF_RAND_VALUES; i++) {
        m_randomValues[i] = static_cast<uint32_t>(rand());
    }

    m_clusterHandler = new ClusterHandler();
    m_segmentQueue = new SegmentQueue();
    m_processingUnitHandler = new ProcessingUnitHandler();
    m_processingUnitHandler->initProcessingUnits(3);
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
    return true;
}

const std::string
KyoukoRoot::initCluster(const std::string &filePath)
{
    LOG_INFO("no files found. Try to create a new cluster");

    LOG_INFO("use init-file: " + filePath);

    std::string fileContent = "";
    std::string errorMessage = "";
    if(Kitsunemimi::Persistence::readFile(fileContent, filePath, errorMessage) == false)
    {
        LOG_ERROR(errorMessage);
        return std::string("");
    }

    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(fileContent == "") {
        return std::string("");
    }

    // parse input
    JsonItem parsedContent;
    const bool ret = parsedContent.parse(fileContent, errorMessage);
    if(ret == false)
    {
        LOG_ERROR("error while parsing input: " + errorMessage);
        return std::string("");
    }

    NetworkCluster* newCluster = new NetworkCluster();
    const std::string uuid = newCluster->initNewCluster(parsedContent);
    if(uuid == "")
    {
        delete newCluster;
        LOG_ERROR("failed to initialize network");
        return std::string("");
    }

    m_clusterHandler->addCluster(uuid, newCluster);

    return uuid;
}
