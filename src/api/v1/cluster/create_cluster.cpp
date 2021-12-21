/**
 * @file        create_cluster.cpp
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

#include "create_cluster.h"

#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiJson/json_item.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

CreateCluster::CreateCluster()
    : Blossom("Create complete new cluster.")
{
    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name for the new cluster.");

    registerInputField("template",
                       SAKURA_STRING_TYPE,
                       true,
                       "Input-file with the definition of the new cluster "
                       "as base64 encoded string.");

    registerOutputField("cluster_uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created cluster.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new created cluster.");
}

bool
CreateCluster::runTask(BlossomLeaf &blossomLeaf,
                       const Kitsunemimi::DataMap &,
                       BlossomStatus &status,
                       Kitsunemimi::ErrorContainer &error)
{
    const std::string input = blossomLeaf.input.get("content").getString();

    DataBuffer resultBuffer;
    if(Kitsunemimi::Crypto::decodeBase64(resultBuffer, input) == false)
    {
        error.addMeesage("base64-decoding of the input failes");
        return false;
    }

    const std::string content = std::string(static_cast<char*>(resultBuffer.data),
                                            resultBuffer.usedBufferSize);

    // parse input
    Kitsunemimi::Json::JsonItem parsedContent;
    if(parsedContent.parse(content, error) == false) {
        return false;
    }

    std::cout<<"###################################################################"<<std::endl;
    std::cout<<parsedContent.toString(true)<<std::endl;

    ClusterInterface* newCluster = new ClusterInterface();
    const std::string uuid = newCluster->initNewCluster(parsedContent);
    if(uuid == "")
    {
        delete newCluster;
        return false;
    }

    KyoukoRoot::m_clusterHandler->addCluster(uuid, newCluster);

    blossomLeaf.output.insert("cluster_uuid", uuid);

    return true;
}
