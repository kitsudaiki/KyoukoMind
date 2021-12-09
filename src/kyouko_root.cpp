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

#include <core/initializing/struct_validation.h>
#include <core/initializing/preprocess_cluster_json.h>

#include <core/processing/cpu/cpu_processing_unit.h>
#include <core/processing/segment_queue.h>
#include <core/processing/processing_unit_handler.h>

#include <core/objects/node.h>
#include <core/storage_io.h>
#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiCommon/files/text_file.h>

#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

// init static variables
ClusterHandler* KyoukoRoot::m_clusterHandler = nullptr;
uint32_t* KyoukoRoot::m_randomValues = nullptr;
SegmentQueue* KyoukoRoot::m_segmentQueue = nullptr;
ProcessingUnitHandler* KyoukoRoot::m_processingUnitHandler = nullptr;

/**
 * @brief KyoukoRoot::KyoukoRoot
 */
KyoukoRoot::KyoukoRoot()
{

}

/**
 * @brief KyoukoRoot::~KyoukoRoot
 */
KyoukoRoot::~KyoukoRoot() {}

/**
 * @brief KyoukoRoot::init
 * @return
 */
bool
KyoukoRoot::init()
{
    validateStructSizes();

    // init predefinde random-values
    m_randomValues = new uint32_t[NUMBER_OF_RAND_VALUES];
    srand(time(NULL));
    for(uint32_t i = 0; i < NUMBER_OF_RAND_VALUES; i++) {
        m_randomValues[i] = static_cast<uint32_t>(rand());
    }

    m_clusterHandler = new ClusterHandler();
    m_segmentQueue = new SegmentQueue();
    m_processingUnitHandler = new ProcessingUnitHandler();
    m_processingUnitHandler->initProcessingUnits(1);

    return true;
}

/**
 * @brief KyoukoRoot::initializeSakuraFiles
 * @return
 */
bool
KyoukoRoot::initializeSakuraFiles(Kitsunemimi::ErrorContainer &error)
{
    bool success = false;
    const std::string sakuraDir = GET_STRING_CONFIG("DEFAULT", "sakura-file-locaion", success);
    if(success == false) {
        return false;
    }

    success = SakuraLangInterface::getInstance()->readFilesInDir(sakuraDir, error);
    if(success == false)
    {
        LOG_ERROR(error);
        return false;
    }

    return true;
}

/**
 * @brief KyoukoRoot::initCluster
 * @param filePath
 * @return
 */
const std::string
KyoukoRoot::initCluster(const std::string &filePath,
                        Kitsunemimi::ErrorContainer &error)
{
    LOG_INFO("no files found. Try to create a new cluster");

    LOG_INFO("use init-file: " + filePath);

    std::string fileContent = "";
    std::string errorMessage = "";
    if(Kitsunemimi::readFile(fileContent, filePath, error) == false)
    {
        LOG_ERROR(error);
        return std::string("");
    }

    // check if values are valid
    if(fileContent == "") {
        return std::string("");
    }

    // parse input
    JsonItem parsedContent;
    const bool ret = parsedContent.parse(fileContent, error);
    if(ret == false)
    {
        LOG_ERROR(error);
        return std::string("");
    }

    ClusterInterface* newCluster = new ClusterInterface();
    const std::string uuid = newCluster->initNewCluster(parsedContent);
    if(uuid == "")
    {
        delete newCluster;
        LOG_ERROR(error);
        return uuid;
    }

    m_clusterHandler->addCluster(uuid, newCluster);

    return uuid;
}
