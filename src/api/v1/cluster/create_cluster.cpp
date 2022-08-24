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

    registerInputField("cluster_definition",
                       SAKURA_MAP_TYPE,
                       false,
                       "Definition, which describe the new cluster.");

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created cluster.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new created cluster.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
CreateCluster::runTask(BlossomLeaf &blossomLeaf,
                       const Kitsunemimi::DataMap &context,
                       BlossomStatus &status,
                       Kitsunemimi::ErrorContainer &error)
{
    const std::string clusterName = blossomLeaf.input.get("name").getString();
    Kitsunemimi::Json::JsonItem clusterDefinition = blossomLeaf.input.get("cluster_definition");
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // check if user already exist within the table
    Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::clustersTable->getClusterByName(getResult,
                                                   clusterName,
                                                   userUuid,
                                                   projectUuid,
                                                   isAdmin,
                                                   error))
    {
        status.errorMessage = "Cluster with name '" + clusterName + "' already exist.";
        error.addMeesage(status.errorMessage);
        status.statusCode = Kitsunemimi::Hanami::CONFLICT_RTYPE;
        return false;
    }

    // convert values
    Kitsunemimi::Json::JsonItem clusterData;
    clusterData.insert("name", clusterName);
    clusterData.insert("project_uuid", "-");
    clusterData.insert("owner_uuid", "-");
    clusterData.insert("visibility", "private");

    // add new user to table
    if(KyoukoRoot::clustersTable->addCluster(clusterData,
                                             userUuid,
                                             projectUuid,
                                             error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to add cluster to database");
        return false;
    }

    // get new created user from database
    if(KyoukoRoot::clustersTable->getClusterByName(blossomLeaf.output,
                                                   clusterName,
                                                   userUuid,
                                                   projectUuid,
                                                   isAdmin,
                                                   error) == false)
    {
        error.addMeesage("Failed to get cluster from database by name '" + clusterName + "'");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    const std::string uuid = blossomLeaf.output.get("uuid").getString();
    Cluster* newCluster = new Cluster();
    if(clusterDefinition.size() != 0)
    {
        if(initCluster(newCluster,
                       uuid,
                       clusterDefinition,
                       context,
                       status,
                       error) == false)
        {
            delete newCluster;
            error.addMeesage("Failed to initialize cluster");
            return false;
        }
    }

    KyoukoRoot::m_clusterHandler->addCluster(uuid, newCluster);

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_uuid");
    blossomLeaf.output.remove("project_uuid");
    blossomLeaf.output.remove("visibility");

    return true;
}

/**
 * @brief CreateCluster::initCluster
 *
 * @param cluster pointer to the cluster, which should be initialized
 * @param clusterUuid uuid of the cluster
 * @param clusterDefinition definition, which describe the new cluster
 * @param context context-object with date for the access to the database-tables
 * @param status reference for status-output
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
CreateCluster::initCluster(Cluster* cluster,
                           const std::string &clusterUuid,
                           Kitsunemimi::Json::JsonItem &clusterDefinition,
                           const Kitsunemimi::DataMap &context,
                           Kitsunemimi::Sakura::BlossomStatus &status,
                           Kitsunemimi::ErrorContainer &error)
{
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

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
        if(getSegmentTemplate(parsedTemplate,
                              type,
                              userUuid,
                              projectUuid,
                              isAdmin,
                              error) == false)
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
 * @param userUuid
 * @param projectUuid
 * @param isAdmin
 * @param error
 * @return
 */
bool
CreateCluster::getSegmentTemplate(Kitsunemimi::Json::JsonItem &parsedTemplate,
                                  const std::string &name,
                                  const std::string &userUuid,
                                  const std::string &projectUuid,
                                  const bool isAdmin,
                                  Kitsunemimi::ErrorContainer &error)
{
    // get segment-template from database
    JsonItem templateData;
    if(KyoukoRoot::templateTable->getTemplateByName(templateData,
                                                    name,
                                                    userUuid,
                                                    projectUuid,
                                                    isAdmin,
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
