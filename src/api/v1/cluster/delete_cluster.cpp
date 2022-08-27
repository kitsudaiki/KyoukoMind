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

#include <core/cluster/cluster_handler.h>
#include <core/cluster/cluster.h>

#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi::Sakura;

DeleteCluster::DeleteCluster()
    : Blossom("Delete a cluster.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "uuid of the cluster.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

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
    // get context-info
    const std::string userId = context.getStringByKey("id");
    const std::string projectId = context.getStringByKey("project_id");
    const bool isAdmin = context.getBoolByKey("is_admin");
    const bool isProjectAdmin = context.getBoolByKey("is_project_admin");

    // get information from request
    const std::string clusterUuid = blossomLeaf.input.get("uuid").getString();

    // check if user exist within the table
    Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::clustersTable->getCluster(getResult,
                                             clusterUuid,
                                             userId,
                                             isAdmin,
                                             projectId,
                                             isProjectAdmin,
                                             error) == false)
    {
        status.errorMessage = "Cluster with uuid '" + clusterUuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        error.addMeesage(status.errorMessage);
        return false;
    }

    // remove data from table
    if(KyoukoRoot::clustersTable->deleteCluster(clusterUuid,
                                                userId,
                                                isAdmin,
                                                projectId,
                                                isProjectAdmin,
                                                error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to delete cluster with UUID '" + clusterUuid + "' from database");
        return false;
    }

    // remove internal data
    const std::string uuid = getResult.get("uuid").getString();
    if(KyoukoRoot::m_clusterHandler->removeCluster(uuid) == false)
    {
        // should never be false, because the uuid is already defined as unique by the database
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to delete cluster with UUID '"
                         + clusterUuid
                         + "' from cluster-handler");
        return false;
    }

    return true;
}
