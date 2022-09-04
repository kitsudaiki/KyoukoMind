/**
 * @file        list_task.cpp
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

#include "list_task.h"

#include <core/cluster/cluster_handler.h>
#include <core/cluster/cluster.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

ListTask::ListTask()
    : Blossom("List all tasks of a cluster.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("cluster_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster, which should process the request");
    assert(addFieldRegex("cluster_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                         "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));


    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("header",
                        SAKURA_ARRAY_TYPE,
                        "Array with the namings all columns of the table.");
    registerOutputField("body",
                        SAKURA_ARRAY_TYPE,
                        "Array with all rows of the table, which array arrays too.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

const std::string
serializeTimePoint(const std::chrono::high_resolution_clock::time_point &time,
                   const std::string &format = "UTC: %Y-%m-%d %H:%M:%S")
{
    std::time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm tm = *std::gmtime(&tt);
    std::stringstream ss;
    ss << std::put_time(&tm, format.c_str() );
    return ss.str();
}

/**
 * @brief runTask
 */
bool
ListTask::runTask(BlossomLeaf &blossomLeaf,
                  const Kitsunemimi::DataMap &context,
                  BlossomStatus &status,
                  Kitsunemimi::ErrorContainer &error)
{
    const Kitsunemimi::Hanami::UserContext userContext(context);
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

    // get progress of all tasks
    std::map<std::string, TaskProgress> progressOverview;
    cluster->getAllProgress(progressOverview);

    // init table-header
    Kitsunemimi::TableItem result;
    result.addColumn("uuid");
    result.addColumn("state");
    result.addColumn("percentage");
    result.addColumn("queued");
    result.addColumn("start");
    result.addColumn("end");

    // build table-content
    std::map<std::string, TaskProgress>::iterator it;
    for(it = progressOverview.begin();
        it != progressOverview.end();
        it++)
    {
        if(it->second.state == QUEUED_TASK_STATE)
        {
            result.addRow(std::vector<std::string>{
                              it->first,
                              "queued",
                              std::to_string(it->second.percentageFinished),
                              serializeTimePoint(it->second.queuedTimeStamp),
                              "-",
                              "-"
                          });
        }
        else if(it->second.state == ACTIVE_TASK_STATE)
        {
            result.addRow(std::vector<std::string>{
                              it->first,
                              "active",
                              std::to_string(it->second.percentageFinished),
                              serializeTimePoint(it->second.queuedTimeStamp),
                              serializeTimePoint(it->second.startActiveTimeStamp),
                              "-"
                          });
        }
        else if(it->second.state == ABORTED_TASK_STATE)
        {
            result.addRow(std::vector<std::string>{
                              it->first,
                              "aborted",
                              std::to_string(it->second.percentageFinished),
                              serializeTimePoint(it->second.queuedTimeStamp),
                              serializeTimePoint(it->second.startActiveTimeStamp),
                              "-"
                          });
        }
        else if(it->second.state == FINISHED_TASK_STATE)
        {
            result.addRow(std::vector<std::string>{
                              it->first,
                              "finished",
                              std::to_string(it->second.percentageFinished),
                              serializeTimePoint(it->second.queuedTimeStamp),
                              serializeTimePoint(it->second.startActiveTimeStamp),
                              serializeTimePoint(it->second.endActiveTimeStamp)
                          });
        }
    }

    // prepare for output
    blossomLeaf.output.insert("header", result.getInnerHeader());
    blossomLeaf.output.insert("body", result.getBody());

    return true;
}
