/**
 * @file        create_request_task.cpp
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

#include "create_request_task.h"
#include <kyouko_root.h>

#include <libSagiriArchive/sagiri_send.h>

#include <core/data_structure/cluster_handler.h>
#include <core/data_structure/cluster.h>

#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;
using Kitsunemimi::Hanami::SupportedComponents;

CreateRequestTask::CreateRequestTask()
    : Blossom("Add new request-task to the task-queue of a cluster.")
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

    // set endpoints for predefined input for dev-mode
    if(DEV_MODE)
    {
        registerInputField("input_data",
                           SAKURA_STRING_TYPE,
                           false,
                           "Input-data.");
    }
    else
    {
        registerInputField("data_set_uuid",
                           SAKURA_STRING_TYPE,
                           true,
                           "UUID to identifiy the train-data with the input in sagiri.");
        assert(addFieldRegex("data_set_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                              "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));
    }

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
CreateRequestTask::runTask(BlossomLeaf &blossomLeaf,
                           const Kitsunemimi::DataMap &context,
                           BlossomStatus &status,
                           Kitsunemimi::ErrorContainer &error)
{
    const std::string clusterUuid = blossomLeaf.input.get("cluster_uuid").getString();
    const std::string dataSetUuid = blossomLeaf.input.get("data_set_uuid").getString();
    const std::string token = context.getStringByKey("token");

    // run dev-mode, if set in config
    if(DEV_MODE)
    {
        const std::string inputData = blossomLeaf.input.get("input_data").getString();

        const std::string taskUuid = testMode(clusterUuid, inputData, error);
        blossomLeaf.output.insert("uuid", taskUuid);
        return true;
    }

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
        status.errorMessage = "cluster with uuid '" + clusterUuid + "'not found";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        error.addMeesage(status.errorMessage);
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

    // get relevant information from output
    const uint64_t numberOfInputs = dataSetInfo.get("inputs").getLong();
    const uint64_t numberOfOutputs = dataSetInfo.get("outputs").getLong();
    const uint64_t numberOfLines = dataSetInfo.get("lines").getLong();

    // get input-data
    DataBuffer* dataSetBuffer = Sagiri::getData(token, dataSetUuid, error);
    if(dataSetBuffer == nullptr)
    {
        error.addMeesage("failed to get data from sagiri for uuid '" + dataSetUuid + "'");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // init request-task
    const std::string taskUuid = cluster->addRequestTask(static_cast<float*>(dataSetBuffer->data),
                                                         numberOfInputs,
                                                         numberOfOutputs,
                                                         numberOfLines);
    cluster->m_segmentCounter = cluster->allSegments.size();
    cluster->updateClusterState();

    blossomLeaf.output.insert("uuid", taskUuid);

    return true;
}

/**
 * @brief run test-mode
 */
const std::string
CreateRequestTask::testMode(const std::string &clusterUuid,
                            const std::string &inputData,
                            Kitsunemimi::ErrorContainer &error)
{

    // get cluster
    Cluster* cluster = KyoukoRoot::m_clusterHandler->getCluster(clusterUuid);
    if(cluster == nullptr)
    {
        error.addMeesage("cluster with uuid not found: " + clusterUuid);
        return "";
    }

    // decode train-data
    DataBuffer inputBuffer;
    Kitsunemimi::Crypto::decodeBase64(inputBuffer, inputData);

    // init learn-task
    //return startMnistTask(cluster, inputBuffer);
    return "";
}
