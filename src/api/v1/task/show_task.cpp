/**
 * @file        show_task.cpp
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

#include "show_task.h"

#include <core/data_structure/cluster_handler.h>
#include <core/data_structure/cluster.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

ShowTask::ShowTask()
    : Blossom("Show information of a specific task.")
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

    registerInputField("cluster_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster, which should process the request");
    assert(addFieldRegex("cluster_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                         "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    registerInputField("with_result",
                       SAKURA_STRING_TYPE,
                       false,
                       "Set to true to also retrun the result of the task.");
    assert(addFieldRegex("with_result", "(true|false)"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("percentage_finished",
                        SAKURA_FLOAT_TYPE,
                        "Percentation of the progress between 0.0 and 1.0.");
    registerOutputField("state",
                        SAKURA_STRING_TYPE,
                        "Actual state of the task."); // TODO: add state-names
    registerOutputField("queue_timestamp",
                        SAKURA_STRING_TYPE,
                        "Timestamp in UTC when the task entered the queued state, "
                        "which is basicall the timestamp when the task was created");
    registerOutputField("start_timestamp",
                        SAKURA_STRING_TYPE,
                        "Timestamp in UTC when the task entered the active state.");
    registerOutputField("end_timestamp",
                        SAKURA_STRING_TYPE,
                        "Timestamp in UTC when the task was finished.");
    registerOutputField("result",
                        SAKURA_ARRAY_TYPE,
                        "Array with results.");

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
ShowTask::runTask(BlossomLeaf &blossomLeaf,
                  const Kitsunemimi::DataMap &context,
                  BlossomStatus &,
                  Kitsunemimi::ErrorContainer &error)
{
    const std::string clusterUuid = blossomLeaf.input.get("cluster_uuid").getString();
    const std::string taskUuid = blossomLeaf.input.get("uuid").getString();
    const bool withResult = blossomLeaf.input.get("with_result").getString() == "true";
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // get cluster
    Cluster* cluster = KyoukoRoot::m_clusterHandler->getCluster(clusterUuid);
    if(cluster == nullptr)
    {
        error.addMeesage("interface with uuid not found: " + clusterUuid);
        return false;
    }

    const TaskProgress progress = cluster->getProgress(taskUuid);

    // get basic information
    blossomLeaf.output.insert("percentage_finished", progress.percentageFinished);
    blossomLeaf.output.insert("queue_timestamp", serializeTimePoint(progress.queuedTimeStamp));

    // get timestamps
    if(progress.state == QUEUED_TASK_STATE)
    {
        blossomLeaf.output.insert("state", "queued");
        blossomLeaf.output.insert("start_timestamp", "-");
        blossomLeaf.output.insert("end_timestamp", "-");
    }
    else if(progress.state == ACTIVE_TASK_STATE)
    {
        blossomLeaf.output.insert("state", "active");
        blossomLeaf.output.insert("start_timestamp",
                                  serializeTimePoint(progress.startActiveTimeStamp));
        blossomLeaf.output.insert("end_timestamp", "-");
    }
    else if(progress.state == ABORTED_TASK_STATE)
    {
        blossomLeaf.output.insert("state", "aborted");
        blossomLeaf.output.insert("start_timestamp",
                                  serializeTimePoint(progress.startActiveTimeStamp));
        blossomLeaf.output.insert("end_timestamp", "-");
    }
    else if(progress.state == FINISHED_TASK_STATE)
    {
        blossomLeaf.output.insert("state", "finished");
        blossomLeaf.output.insert("start_timestamp",
                                  serializeTimePoint(progress.startActiveTimeStamp));
        blossomLeaf.output.insert("end_timestamp", serializeTimePoint(progress.endActiveTimeStamp));
    }

    // get task-result
    if(withResult)
    {
        const uint32_t* resultData = cluster->getResultData(taskUuid);
        const uint32_t resultSize = cluster->getResultSize(taskUuid);

        Kitsunemimi::DataArray* results = new Kitsunemimi::DataArray();

        for(uint32_t i = 0; i < resultSize; i++) {
            results->append(new DataValue(static_cast<int>(resultData[i])));
        }

        blossomLeaf.output.insert("result", results);
    }

    return true;
}
