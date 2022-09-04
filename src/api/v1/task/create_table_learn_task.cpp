/**
 * @file        create_table_learn_task.cpp
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

#include "create_table_learn_task.h"
#include <kyouko_root.h>
#include <core/cluster/cluster_handler.h>
#include <core/cluster/cluster.h>

#include <libSagiriArchive/datasets.h>

#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiNetwork/hanami_messaging.h>

#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;
using Kitsunemimi::Hanami::SupportedComponents;

CreateTableLearnTask::CreateTableLearnTask()
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
CreateTableLearnTask::runTask(BlossomLeaf &blossomLeaf,
                              const Kitsunemimi::DataMap &context,
                              BlossomStatus &status,
                              Kitsunemimi::ErrorContainer &error)
{
    const std::string clusterUuid = blossomLeaf.input.get("cluster_uuid").getString();
    const std::string dataSetUuid = blossomLeaf.input.get("data_set_uuid").getString();
    const std::string columnName = blossomLeaf.input.get("column_name").getString();
    const Kitsunemimi::Hanami::UserContext userContext(context);

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
        status.errorMessage = "Cluster with UUID '" + clusterUuid + "'not found";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        error.addMeesage(status.errorMessage);
        return false;
    }

    // get meta-infos of data-set from sagiri
    Kitsunemimi::Json::JsonItem dataSetInfo;
    if(Sagiri::getDataSetInformation(dataSetInfo, dataSetUuid, userContext.token, error) == false)
    {
        error.addMeesage("Failed to get information from sagiri for dataset with UUID '"
                         + dataSetUuid
                         + "'");
        // TODO: add status-error from response from sagiri
        status.statusCode = Kitsunemimi::Hanami::UNAUTHORIZED_RTYPE;
        return false;
    }

    // get input-data
    DataBuffer* colBuffer = Sagiri::getDatasetData(userContext.token,
                                                   dataSetUuid,
                                                   columnName,
                                                   error);
    if(colBuffer == nullptr)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Got NO data from sagiri for dataset with UUID '" + dataSetUuid + "'");
        return false;
    }

    // create task
    const uint64_t numberOfLines = dataSetInfo.get("lines").getLong();
    const std::string taskUuid = cluster->addTableLearnTask(static_cast<float*>(colBuffer->data),
                                                            numberOfLines,
                                                            numberOfLines - 100);

    colBuffer->data = nullptr;
    delete colBuffer;

    blossomLeaf.output.insert("uuid", taskUuid);

    return true;
}

