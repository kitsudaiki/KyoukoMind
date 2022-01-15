/**
 * @file        delete_cluster.cpp
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

#include "delete_cluster.h"

#include <kyouko_root.h>

#include <core/data_structure/cluster_handler.h>
#include <core/data_structure/cluster.h>

#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi::Sakura;

DeleteCluster::DeleteCluster()
    : Blossom("Delete a cluster.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the cluster.");
    // column in database is limited to 256 characters size
    assert(addFieldBorder("name", 4, 256));
    assert(addFieldRegex("name", "[a-zA-Z][a-zA-Z_0-9]*"));

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
DeleteCluster::runTask(BlossomLeaf &blossomLeaf,
                       const Kitsunemimi::DataMap &context,
                       BlossomStatus &status,
                       Kitsunemimi::ErrorContainer &error)
{
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // get information from request
    const std::string clusterName = blossomLeaf.input.get("name").getString();

    // check if user exist within the table
    Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::clustersTable->getClusterByName(getResult,
                                                   clusterName, userUuid,
                                                   projectUuid,
                                                   isAdmin,
                                                   error) == false)
    {
        status.errorMessage = "Cluster with name '" + clusterName + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // remove data from table
    if(KyoukoRoot::clustersTable->deleteCluster(clusterName,
                                                userUuid,
                                                projectUuid,
                                                isAdmin,
                                                error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // remove internal data
    const std::string uuid = getResult.get("uuid").getString();
    if(KyoukoRoot::m_clusterHandler->removeCluster(uuid) == false)
    {
        // should never be false, because the uuid is already defined as unique by the database
        return false;
    }

    return true;
}
