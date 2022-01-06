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

#include "create_cluster.h"

#include <core/data_structure/cluster_handler.h>
#include <core/data_structure/cluster.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiJson/json_item.h>

#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

CreateCluster::CreateCluster()
    : Blossom("Create complete new cluster.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name for the new cluster.");
    // column in database is limited to 256 characters size
    assert(addFieldBorder("name", 4, 256));
    assert(addFieldRegex("name", "[a-zA-Z][a-zA-Z_0-9]*"));

    registerInputField("template_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the template, which should be used as base for the cluster.");
    assert(addFieldRegex("template_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                          "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created cluster.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new created cluster.");
    registerOutputField("template_uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the template, which should be used as base for the cluster.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
CreateCluster::runTask(BlossomLeaf &blossomLeaf,
                       const Kitsunemimi::DataMap &,
                       BlossomStatus &status,
                       Kitsunemimi::ErrorContainer &error)
{
    const std::string clusterName = blossomLeaf.input.get("name").getString();
    const std::string templateUuid = blossomLeaf.input.get("template_uuid").getString();

    // check if user already exist within the table
    Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::clustersTable->getClusterByName(getResult, clusterName, error))
    {
        status.errorMessage = "Cluster with name '" + clusterName + "' already exist.";
        status.statusCode = Kitsunemimi::Hanami::CONFLICT_RTYPE;
        return false;
    }

    // get new created user from database
    JsonItem templateData;
    if(KyoukoRoot::templateTable->getTemplate(templateData,
                                              templateUuid,
                                              error,
                                              true) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        status.errorMessage = "Template with UUID '" + templateUuid + "' doesn't exist";
        return false;
    }

    // decode template
    std::string contentDecoded;
    if(Kitsunemimi::Crypto::decodeBase64(contentDecoded,
                                         templateData.get("data").getString()) == false)
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
    clusterData.insert("name", clusterName);
    clusterData.insert("template_uuid", templateUuid);
    clusterData.insert("project_uuid", "-");
    clusterData.insert("owner_uuid", "-");
    clusterData.insert("visibility", 0);

    // add new user to table
    if(KyoukoRoot::clustersTable->addCluster(clusterData, error) == false)
    {
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
    Cluster* newCluster = new Cluster();
    if(newCluster->init(parsedContent, uuid) == false)
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
