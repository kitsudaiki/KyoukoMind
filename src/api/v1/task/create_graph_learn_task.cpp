/**
 * @file        create_graph_learn_task.cpp
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

#include "create_graph_learn_task.h"
#include <kyouko_root.h>

#include <libSagiriArchive/sagiri_send.h>

#include <core/cluster/cluster_handler.h>
#include <core/cluster/cluster.h>

#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;
using Kitsunemimi::Hanami::SupportedComponents;

CreateGraphLearnTask::CreateGraphLearnTask()
    : Blossom("Add new learn-task to the task-queue of a cluster.")
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

    registerInputField("data_set_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID to identifiy the train-data with the input in sagiri.");
    assert(addFieldRegex("data_set_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                          "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    registerInputField("column_name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the column of the table, which should work as input");

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created task.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
CreateGraphLearnTask::runTask(BlossomLeaf &blossomLeaf,
                              const Kitsunemimi::DataMap &context,
                              BlossomStatus &status,
                              Kitsunemimi::ErrorContainer &error)
{
    const std::string clusterUuid = blossomLeaf.input.get("cluster_uuid").getString();
    const std::string dataSetUuid = blossomLeaf.input.get("data_set_uuid").getString();
    const std::string columnName = blossomLeaf.input.get("column_name").getString();

    const std::string token = context.getStringByKey("token");

    // check if sagiri is available
    SupportedComponents* scomp = SupportedComponents::getInstance();
    if(scomp->support[Kitsunemimi::Hanami::SAGIRI] == false)
    {
        status.statusCode = Kitsunemimi::Hanami::SERVICE_UNAVAILABLE_RTYPE;
        status.errorMessage = "Sagiri is not configured for Kyouko.";
        error.addMeesage(status.errorMessage);
        return false;
    }

    // get cluster
    Cluster* cluster = KyoukoRoot::m_clusterHandler->getCluster(clusterUuid);
    if(cluster == nullptr)
    {
        error.addMeesage("interface with uuid not found: " + clusterUuid);
        return false;
    }

    // get meta-infos of data-set from sagiri
    Kitsunemimi::Json::JsonItem dataSetInfo;
    if(Sagiri::getDataSetInformation(dataSetInfo, dataSetUuid, token, error) == false)
    {
        error.addMeesage("failed to get information from sagiri for uuid '" + dataSetUuid + "'");
        // TODO: add status-error from response from sagiri
        status.statusCode = Kitsunemimi::Hanami::UNAUTHORIZED_RTYPE;
        return false;
    }

    // get input-data
    DataBuffer* dataSetBuffer = Sagiri::getData(token, dataSetUuid, error);
    if(dataSetBuffer == nullptr)
    {
        error.addMeesage("failed to get data from sagiri");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // create task
    const uint64_t numberOfLines = dataSetInfo.get("lines").getLong();
    const std::string taskUuid = cluster->addGraphLearnTask(static_cast<float*>(dataSetBuffer->data),
                                                            numberOfLines - 5001);

    blossomLeaf.output.insert("uuid", taskUuid);

    return true;
}
