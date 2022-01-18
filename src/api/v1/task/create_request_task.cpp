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

    registerInputField("type",
                       SAKURA_STRING_TYPE,
                       true,
                       "Type of the data (MNIST, CSV).");
    assert(addFieldRegex("type", "(csv|mnist)"));

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
        registerInputField("input_data_uuid",
                           SAKURA_STRING_TYPE,
                           true,
                           "UUID to identifiy the train-data with the input in sagiri.");
        assert(addFieldRegex("input_data_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
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


bool
getDataFromSagiri(DataBuffer &result,
                  const std::string &token,
                  const std::string &uuid,
                  Kitsunemimi::ErrorContainer &error)
{
    Kitsunemimi::Hanami::ResponseMessage response;
    Kitsunemimi::Hanami::HanamiMessaging* msg = Kitsunemimi::Hanami::HanamiMessaging::getInstance();

    Kitsunemimi::Hanami::RequestMessage request;
    request.id = "v1/train_data";
    request.httpType = Kitsunemimi::Hanami::GET_TYPE;


    request.inputValues = "{\"token\":\"" + token + "\""
                          ",\"uuid\":\"" + uuid + "\""
                          ",\"with_data\":true}";
    if(msg->triggerSakuraFile("sagiri", response, request, error) == false) {
        return false;
    }

    if(response.success == false)
    {
        error.addMeesage(response.responseContent);
        return false;
    }

    // parse result
    Kitsunemimi::Json::JsonItem jsonItem;
    if(jsonItem.parse(response.responseContent, error) == false) {
        return false;
    }

    // get input-data
    if(Kitsunemimi::Crypto::decodeBase64(result, jsonItem.get("data").getString()) == false)
    {
        error.addMeesage("base64-decoding of the input failes");
        return false;
    }

    return true;
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
    const std::string inputUuid = blossomLeaf.input.get("input_data_uuid").getString();
    const std::string type = blossomLeaf.input.get("type").getString();
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

    // get input-data
    DataBuffer inputBuffer;
    if(getDataFromSagiri(inputBuffer, token, inputUuid, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // init request-task
    std::string taskUuid = "";
    if(type == "mnist") {
        taskUuid = startMnistTask(cluster, inputBuffer);
    }

    blossomLeaf.output.insert("uuid", taskUuid);

    return true;
}

/**
 * @brief start mnist-task
 *
 * @param cluster pointer to cluster
 * @param inputBuffer buffer with input-data
 *
 * @return task-uuid
 */
const std::string
CreateRequestTask::startMnistTask(Cluster* cluster,
                                  const Kitsunemimi::DataBuffer &inputBuffer)
{
    uint8_t* dataBufferPtr = static_cast<uint8_t*>(inputBuffer.data);

    // get number of images
    uint32_t numberOfImages = 0;
    numberOfImages |= dataBufferPtr[7];
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[6]) << 8;
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[5]) << 16;
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[4]) << 24;

    // get number of rows
    uint32_t numberOfRows = 0;
    numberOfRows |= dataBufferPtr[11];
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[10]) << 8;
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[9]) << 16;
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[8]) << 24;

    // get number of columns
    uint32_t numberOfColumns = 0;
    numberOfColumns |= dataBufferPtr[15];
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[14]) << 8;
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[13]) << 16;
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[12]) << 24;

    // get pictures
    const uint32_t pictureSize = numberOfRows * numberOfColumns;

    uint64_t dataPos = 0;
    uint64_t dataSize = numberOfImages * pictureSize;
    float* taskData = new float[dataSize];

    for(uint32_t pic = 0; pic < numberOfImages; pic++)
    {
        // input
        for(uint32_t i = 0; i < pictureSize; i++)
        {
            const uint32_t pos = pic * pictureSize + i + 16;
            taskData[dataPos] = (static_cast<float>(dataBufferPtr[pos]) / 255.0f);
            dataPos++;
        }
    }

    // create task
    const std::string taskUuid = cluster->addRequestTask(taskData,
                                                         pictureSize,
                                                         numberOfImages);
    cluster->m_segmentCounter = cluster->allSegments.size();
    cluster->updateClusterState();

    return taskUuid;
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
    return startMnistTask(cluster, inputBuffer);
}
