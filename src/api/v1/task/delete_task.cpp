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
                       "UUID of the task, which should be deleted");
    assert(addFieldRegex("uuid", UUID_REGEX));

    registerInputField("cluster_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster, which contains the task in its queue");
    assert(addFieldRegex("cluster_uuid", UUID_REGEX));

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
                    BlossomStatus &status,
                    Kitsunemimi::ErrorContainer &error)
{
    const Kitsunemimi::Hanami::UserContext userContext(context);
    const std::string taskUuid = blossomLeaf.input.get("uuid").getString();
    const std::string clusterUuid = blossomLeaf.input.get("cluster_uuid").getString();

    // get cluster
    Cluster* cluster = KyoukoRoot::m_clusterHandler->getCluster(clusterUuid);
    if(cluster == nullptr)
    {
        status.errorMessage = "Cluster with UUID '" + clusterUuid + "'not found";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        error.addMeesage(status.errorMessage);
        return false;
    }

    // delete task
    if(cluster->removeTask(taskUuid) == false)
    {
        status.errorMessage = "Task with UUID '" + clusterUuid + "'not found in "
                              "Cluster with UUID '" + clusterUuid;
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        error.addMeesage(status.errorMessage);
        return false;
    }

    return true;
}
