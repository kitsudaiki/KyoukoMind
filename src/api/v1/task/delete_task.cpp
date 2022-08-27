/**
 * @file        delete_task.cpp
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

#include "delete_task.h"

#include <core/cluster/cluster_handler.h>
#include <core/cluster/cluster.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

DeleteTask::DeleteTask()
    : Blossom("Delete a task or abort a task, if it is actually running.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster, which should process the request");
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
DeleteTask::runTask(BlossomLeaf &blossomLeaf,
                    const Kitsunemimi::DataMap &context,
                    BlossomStatus &,
                    Kitsunemimi::ErrorContainer &)
{
    const std::string uuid = blossomLeaf.input.get("uuid").getString();

    const std::string userId = context.getStringByKey("id");
    const std::string projectId = context.getStringByKey("project_id");
    const bool isAdmin = context.getBoolByKey("is_admin");
    const bool isProjectAdmin = context.getBoolByKey("is_project_admin");

    // get cluster
    Cluster* cluster = KyoukoRoot::m_clusterHandler->getCluster(uuid);
    if(cluster == nullptr)
    {
        //error.addMeesage("interface with uuid not found: " + uuid);
        return true;
    }

    return true;
}
