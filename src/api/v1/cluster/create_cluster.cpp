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

#include <core/cluster/cluster_handler.h>
#include <core/cluster/cluster.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiCommon/structs.h>

#include <libKitsunemimiCrypto/common.h>
#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiJson/json_item.h>

#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

CreateCluster::CreateCluster()
    : Blossom("Create new cluster.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name for the new cluster.");
    assert(addFieldBorder("name", 4, 256));
    assert(addFieldRegex("name", NAME_REGEX));

    registerInputField("cluster_definition",
                       SAKURA_MAP_TYPE,
                       false,
                       "Json-string, which describe the structure of the new cluster.");

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created cluster.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new created cluster.");
    registerOutputField("owner_id",
                        SAKURA_STRING_TYPE,
                        "ID of the user, who created the new cluster.");
    registerOutputField("project_id",
                        SAKURA_STRING_TYPE,
                        "ID of the project, where the new cluster belongs to.");
    registerOutputField("visibility",
                        SAKURA_STRING_TYPE,
                        "Visibility of the new created cluster (private, shared, public).");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
CreateCluster::runTask(BlossomIO &blossomIO,
                       const Kitsunemimi::DataMap &context,
                       BlossomStatus &status,
                       Kitsunemimi::ErrorContainer &error)
{
    const std::string clusterName = blossomIO.input.get("name").getString();
    Kitsunemimi::Json::JsonItem clusterDefinition = blossomIO.input.get("cluster_definition");
    const Kitsunemimi::Hanami::UserContext userContext(context);

    // check if user already exist within the table
    Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::clustersTable->getClusterByName(getResult, clusterName, userContext, error))
    {
        status.errorMessage = "Cluster with name '" + clusterName + "' already exist.";
        error.addMeesage(status.errorMessage);
        status.statusCode = Kitsunemimi::Hanami::CONFLICT_RTYPE;
        return false;
    }

    // convert values
    Kitsunemimi::Json::JsonItem clusterData;
    clusterData.insert("name", clusterName);
    clusterData.insert("project_id", userContext.projectId);
    clusterData.insert("owner_id", userContext.userId);
    clusterData.insert("visibility", "private");

    // add new user to table
    if(KyoukoRoot::clustersTable->addCluster(clusterData, userContext, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to add cluster to database");
        return false;
    }

    // get new created user from database
    if(KyoukoRoot::clustersTable->getClusterByName(blossomIO.output,
                                                   clusterName,
                                                   userContext,
                                                   error) == false)
    {
        error.addMeesage("Failed to get cluster from database by name '" + clusterName + "'");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    const std::string uuid = blossomIO.output.get("uuid").getString();
    Cluster* newCluster = new Cluster();
    if(clusterDefinition.size() != 0)
    {
        if(initCluster(newCluster, uuid, clusterDefinition, userContext, status, error) == false)
        {
            delete newCluster;
            error.addMeesage("Failed to initialize cluster");
            return false;
        }
    }

    KyoukoRoot::m_clusterHandler->addCluster(uuid, newCluster);

    // remove irrelevant fields
    blossomIO.output.remove("owner_id");
    blossomIO.output.remove("project_id");
    blossomIO.output.remove("visibility");

    return true;
}

/**
 * @brief CreateCluster::initCluster
 *
 * @param cluster pointer to the cluster, which should be initialized
 * @param clusterUuid uuid of the cluster
 * @param clusterDefinition definition, which describe the new cluster
 * @param userContext context-object with date for the access to the database-tables
 * @param status reference for status-output
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
CreateCluster::initCluster(Cluster* cluster,
                           const std::string &clusterUuid,
                           Kitsunemimi::Json::JsonItem &clusterDefinition,
                           const Kitsunemimi::Hanami::UserContext &userContext,
                           Kitsunemimi::Sakura::BlossomStatus &status,
                           Kitsunemimi::ErrorContainer &error)
{
    // collect all segment-templates, which are required by the cluster-template
    Kitsunemimi::Json::JsonItem segments = clusterDefinition.get("segments");
    std::map<std::string, Kitsunemimi::Json::JsonItem> segmentTemplates;
    for(uint64_t i = 0; i < segments.size(); i++)
    {
        const std::string type = segments.get(i).get("type").getString();

        // skip input- and output-segments, because they are generated anyway
        if(type == "input"
                || type == "output")
        {
            continue;
        }

        // get the content of the segment-template
        Kitsunemimi::Json::JsonItem parsedTemplate;
        if(getSegmentTemplate(parsedTemplate, type, userContext, error) == false)
        {
            // TODO: set status-message and maybe change to not-found-error
            status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
            return false;
        }

        // add segment-template to a map, which is generated later when creating the segments
        // based on these templates
        const std::string name = segments.get(i).get("name").getString();
        segmentTemplates.emplace(name, parsedTemplate);
    }

    // generate and initialize the cluster based on the cluster- and segment-templates
    if(cluster->init(clusterDefinition, segmentTemplates, clusterUuid) == false)
    {
        error.addMeesage("Failed to initialize cluster based on a template");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    return true;
}

/**
 * @brief CreateCluster::getSegmentTemplate
 * @param parsedTemplate
 * @param name
 * @param userId
 * @param projectId
 * @param isAdmin
 * @param error
 * @return
 */
bool
CreateCluster::getSegmentTemplate(Kitsunemimi::Json::JsonItem &parsedTemplate,
                                  const std::string &name,
                                  const Kitsunemimi::Hanami::UserContext &userContext,
                                  Kitsunemimi::ErrorContainer &error)
{
    // get segment-template from database
    JsonItem templateData;
    if(KyoukoRoot::templateTable->getTemplateByName(templateData,
                                                    name,
                                                    userContext,
                                                    error,
                                                    true) == false)
    {
        return false;
    }

    // decode template
    std::string decodedTemplate = "";
    if(Kitsunemimi::Crypto::decodeBase64(decodedTemplate,
                                         templateData.get("data").getString()) == false)
    {
        // TODO: better error-messages with uuid
        error.addMeesage("base64-decoding of the template failes");
        return false;
    }

    // parse json-formated-template
    if(parsedTemplate.parse(decodedTemplate, error) == false)
    {
        error.addMeesage("Failed to parse decoded template");
        return false;
    }

    return true;
}
