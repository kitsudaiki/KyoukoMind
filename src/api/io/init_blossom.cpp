/**
 * @file        init_blossom.cpp
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

#include "init_blossom.h"
#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>

#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiJson/json_item.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

InitBlossom::InitBlossom()
    : Blossom()
{
    registerInputField("content", true);

    registerOutputField("cluster_uuid");
}

/**
 * @brief InitBlossom::runTask
 * @param blossomLeaf
 * @param errorMessage
 * @return
 */
bool
InitBlossom::runTask(BlossomLeaf &blossomLeaf,
                     const Kitsunemimi::DataMap &,
                     BlossomStatus &status,
                     Kitsunemimi::ErrorContainer &error)
{
    const std::string content = blossomLeaf.input.getStringByKey("content");

    // parse input
    Kitsunemimi::Json::JsonItem parsedContent;
    const bool ret = parsedContent.parse(content, error);
    if(ret == false) {
        return false;
    }

    ClusterInterface* newCluster = new ClusterInterface();
    const std::string uuid = newCluster->initNewCluster(parsedContent);
    if(uuid == "")
    {
        delete newCluster;
        return false;
    }

    KyoukoRoot::m_clusterHandler->addCluster(uuid, newCluster);

    blossomLeaf.output.insert("cluster_uuid", new Kitsunemimi::DataValue(uuid));

    return true;
}
