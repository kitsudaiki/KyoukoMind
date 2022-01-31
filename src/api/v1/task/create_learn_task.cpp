/**
 * @file        create_learn_task.cpp
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

#include "create_learn_task.h"
#include <kyouko_root.h>

#include <core/data_structure/cluster_handler.h>
#include <core/data_structure/cluster.h>

#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;
using Kitsunemimi::Hanami::SupportedComponents;

CreateLearnTask::CreateLearnTask()
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


    // set endpoints for predefined input for dev-mode
    if(DEV_MODE)
    {
        registerInputField("data_set_uuid",
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

DataBuffer*
getData(const std::string &token,
        const std::string &uuid,
        Kitsunemimi::ErrorContainer &error)
{
    Kitsunemimi::Hanami::ResponseMessage response;
    Kitsunemimi::Hanami::HanamiMessaging* msg = Kitsunemimi::Hanami::HanamiMessaging::getInstance();

    // build request to get train-data from sagiri
    Kitsunemimi::Hanami::RequestMessage request;
    request.id = "v1/data_set";
    request.httpType = Kitsunemimi::Hanami::GET_TYPE;
    request.inputValues = "{\"token\":\"" + token + "\""
                          ",\"uuid\":\"" + uuid + "\""
                          "}";

    // send request to sagiri
    if(msg->triggerSakuraFile("sagiri", response, request, error) == false) {
        return nullptr;
    }

    // check response
    if(response.success == false)
    {
        error.addMeesage(response.responseContent);
        return nullptr;
    }

    // parse result
    Kitsunemimi::Json::JsonItem jsonItem;
    if(jsonItem.parse(response.responseContent, error) == false) {
        return nullptr;
    }

    const std::string location = jsonItem.get("location").getString();
    const std::string message = "{\"location\":\"" + location + "\"}";

    return  msg->sendGenericMessage("sagiri", message.c_str(), message.size(), error);
}

/**
 * @brief runTask
 */
bool
CreateLearnTask::runTask(BlossomLeaf &blossomLeaf,
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
        const std::string labelData = blossomLeaf.input.get("label_data").getString();

        const std::string taskUuid = testMode(clusterUuid, inputData, labelData, error);
        blossomLeaf.output.insert("uuid", taskUuid);
        return true;
    }

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

    // get input-data
    DataBuffer* dataSetBuffer = getData(token, dataSetUuid, error);
    if(dataSetBuffer == nullptr)
    {
        error.addMeesage("failed to get data from sagiri");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // create task
    const std::string taskUuid = cluster->addLearnTask(static_cast<float*>(dataSetBuffer->data),
                                                       784,
                                                       10,
                                                       60000);
    cluster->m_segmentCounter = cluster->allSegments.size();
    cluster->updateClusterState();

    blossomLeaf.output.insert("uuid", taskUuid);

    return true;
}

/**
 * @brief run test-mode
 */
const std::string
CreateLearnTask::testMode(const std::string &clusterUuid,
                          const std::string &inputData,
                          const std::string &labelData,
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
    DataBuffer labelBuffer;
    Kitsunemimi::Crypto::decodeBase64(labelBuffer, labelData);

    // init learn-task
    //return startMnistTask(cluster, inputBuffer);
    return "";
}
