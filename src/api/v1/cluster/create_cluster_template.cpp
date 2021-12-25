/**
 * @file        create_cluster_template.cpp
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

#include "create_cluster_template.h"

#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiJson/json_item.h>

#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

CreateClusterTemplate::CreateClusterTemplate()
    : Blossom("Create complete new cluster.")
{
    // input
    registerInputField("cluster_name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name for the new cluster.");
    // column in database is limited to 256 characters size
    assert(addFieldBorder("cluster_name", 4, 256));
    assert(addFieldRegex("cluster_name", "[a-zA-Z][a-zA-Z_0-9]*"));

    registerInputField("template",
                       SAKURA_STRING_TYPE,
                       true,
                       "Input-file with the definition of the new cluster "
                       "as base64 encoded string.");

    // output
    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created cluster.");
    registerOutputField("cluster_name",
                        SAKURA_STRING_TYPE,
                        "Name of the new created cluster.");
}

/**
 * @brief CreateCluster::runTask
 * @param blossomLeaf
 * @param status
 * @param error
 * @return
 */
bool
CreateClusterTemplate::runTask(BlossomLeaf &blossomLeaf,
                               const Kitsunemimi::DataMap &,
                               BlossomStatus &status,
                               Kitsunemimi::ErrorContainer &error)
{
    const std::string clusterName = blossomLeaf.input.get("cluster_name").getString();
    const std::string content = blossomLeaf.input.get("template").getString();

    // check if user already exist within the table
    Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::clustersTable->getClusterByName(getResult, clusterName, error))
    {
        status.errorMessage = "Cluster with name '" + clusterName + "' already exist.";
        status.statusCode = Kitsunemimi::Hanami::CONFLICT_RTYPE;
        return false;
    }

    // decode template
    std::string contentDecoded;
    if(Kitsunemimi::Crypto::decodeBase64(contentDecoded, content) == false)
    {
        error.addMeesage("base64-decoding of the input failes");
        status.statusCode = Kitsunemimi::Hanami::BAD_REQUEST_RTYPE;
        status.errorMessage = "Given template is not a valid base64 string";
        return false;
    }

    // parse template
    Kitsunemimi::Json::JsonItem parsedContent;
    if(parsedContent.parse(contentDecoded, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // convert values
    Kitsunemimi::Json::JsonItem clusterData;
    clusterData.insert("cluster_name", clusterName);
    clusterData.insert("template", content);

    // add new user to table
    if(KyoukoRoot::clustersTable->addCluster(clusterData, error) == false)
    {
        status.errorMessage = error.toString();
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // get new created user from database
    if(KyoukoRoot::clustersTable->getClusterByName(blossomLeaf.output, clusterName, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    const std::string uuid = blossomLeaf.output.get("uuid").getString();
    ClusterInterface* newCluster = new ClusterInterface();
    if(newCluster->initNewCluster(parsedContent, uuid))
    {
        delete newCluster;
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    KyoukoRoot::m_clusterHandler->addCluster(uuid, newCluster);

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_uuid");
    blossomLeaf.output.remove("project_uuid");
    blossomLeaf.output.remove("visibility");

    return true;
}
