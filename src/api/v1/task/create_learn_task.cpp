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

#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>

#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;
using Kitsunemimi::Hanami::SupportedComponents;

CreateLearnTask::CreateLearnTask()
    : Blossom("Add new learn-task to the task-queue of a cluster.")
{
    registerInputField("cluster_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster, which should process the request");
    registerInputField("input_data_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID to identifiy the train-data with the input in sagiri.");
    registerInputField("label_data_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID to identifiy the train-data with the labels in sagiri.");
    registerInputField("type",
                       SAKURA_STRING_TYPE,
                       true,
                       "Type of the data (MNIST, CSV).");


    registerInputField("input_data",
                       SAKURA_STRING_TYPE,
                       false,
                       "Type of the data (MNIST, CSV).");
    registerInputField("label_data",
                       SAKURA_STRING_TYPE,
                       false,
                       "Type of the data (MNIST, CSV).");

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created task.");
}

bool
getData(DataBuffer &result,
        const std::string &token,
        const std::string &uuid,
        Kitsunemimi::ErrorContainer &error)
{
    Kitsunemimi::Hanami::ResponseMessage response;
    Kitsunemimi::Hanami::HanamiMessaging* msg = Kitsunemimi::Hanami::HanamiMessaging::getInstance();

    Kitsunemimi::Hanami::RequestMessage request;
    request.id = "v1/train_data";
    request.httpType = Kitsunemimi::Hanami::GET_TYPE;


    request.inputValues = "{ \"token\" : \"" + token + "\", \"uuid\" : \"" + uuid + "\",\"with_data\":true}";
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
 * @brief CreateLearnTask::runTask
 * @param blossomLeaf
 * @param status
 * @param error
 * @return
 */
bool
CreateLearnTask::runTask(BlossomLeaf &blossomLeaf,
                         const Kitsunemimi::DataMap &context,
                         BlossomStatus &status,
                         Kitsunemimi::ErrorContainer &error)
{
    const std::string uuid = blossomLeaf.input.get("cluster_uuid").getString();
    const std::string inputUuid = blossomLeaf.input.get("input_data_uuid").getString();
    const std::string labelUuid = blossomLeaf.input.get("label_data_uuid").getString();
    const std::string type = blossomLeaf.input.get("type").getString();
    const std::string token = context.getStringByKey("token");

    const std::string inputData = blossomLeaf.input.get("input_data").getString();
    const std::string labelData = blossomLeaf.input.get("label_data").getString();


    SupportedComponents* scomp = SupportedComponents::getInstance();

    if(inputData == ""
            || labelData == "")
    {
        if(scomp->support[Kitsunemimi::Hanami::SAGIRI] == false)
        {
            status.statusCode = Kitsunemimi::Hanami::SERVICE_UNAVAILABLE_RTYPE;
            status.errorMessage = "Sagiri is not configured for Kyouko.";
            error.addMeesage(status.errorMessage);
            return false;
        }
    }

    // get cluster
    ClusterInterface* cluster = KyoukoRoot::m_clusterHandler->getCluster(uuid);
    if(cluster == nullptr)
    {
        error.addMeesage("interface with uuid not found: " + uuid);
        return false;
    }

    // get input-data
    DataBuffer inputBuffer;
    if(inputData == "")
    {
        if(getData(inputBuffer, token, inputUuid, error) == false)
        {
            status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
            return false;
        }
    }
    else
    {
        Kitsunemimi::Crypto::decodeBase64(inputBuffer, inputData);
    }

    // get label-data
    DataBuffer labelBuffer;
    if(labelData == "")
    {
        if(getData(labelBuffer, token, labelUuid, error) == false)
        {
            status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
            return false;
        }
    }
    else
    {
        Kitsunemimi::Crypto::decodeBase64(labelBuffer, labelData);
    }

    // init learn-task
    std::string taskUuid = "";
    if(type == "mnist") {
        taskUuid = startMnistTask(cluster, inputBuffer, labelBuffer);
    }

    blossomLeaf.output.insert("uuid", taskUuid);

    return true;
}

/**
 * @brief CreateLearnTask::startMnistTask
 * @param cluster
 * @param inputBuffer
 * @param labelBuffer
 * @return
 */
const std::string
CreateLearnTask::startMnistTask(ClusterInterface* cluster,
                                const DataBuffer &inputBuffer,
                                const DataBuffer &labelBuffer)
{
    uint8_t* dataBufferPtr = static_cast<uint8_t*>(inputBuffer.data);
    uint8_t* labelBufferPtr = static_cast<uint8_t*>(labelBuffer.data);

    // get number of images
    uint32_t numberOfImages = 0;
    numberOfImages |= dataBufferPtr[7];
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[6]) << 8;
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[5]) << 16;
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[4]) << 24;
    std::cout<<"number of images: "<<numberOfImages<<std::endl;

    // get number of rows
    uint32_t numberOfRows = 0;
    numberOfRows |= dataBufferPtr[11];
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[10]) << 8;
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[9]) << 16;
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[8]) << 24;
    std::cout<<"number of rows: "<<numberOfRows<<std::endl;

    // get number of columns
    uint32_t numberOfColumns = 0;
    numberOfColumns |= dataBufferPtr[15];
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[14]) << 8;
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[13]) << 16;
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[12]) << 24;
    std::cout<<"number of columns: "<<numberOfColumns<<std::endl;

    // get pictures
    const uint32_t pictureSize = numberOfRows * numberOfColumns;

    std::cout<<"learn"<<std::endl;

    uint64_t dataPos = 0;
    uint64_t dataSize = numberOfImages * pictureSize;
    float* taskData = new float[dataSize];



    uint64_t labelPos = 0;
    uint64_t labelSize = numberOfImages * 10;
    float* labelData = new float[labelSize];


    for(uint32_t pic = 0; pic < numberOfImages; pic++)
    {
        // input
        for(uint32_t i = 0; i < pictureSize; i++)
        {
            const uint32_t pos = pic * pictureSize + i + 16;
            taskData[dataPos] = (static_cast<float>(dataBufferPtr[pos]) / 255.0f);
            dataPos++;
        }

        // output
        for(uint32_t i = 0; i < 10; i++)
        {
            labelData[labelPos] = 0.0f;
            labelPos++;
        }
        const uint32_t label = labelBufferPtr[pic + 8];
        labelData[(labelPos - 10) + label] = 1.0f;
    }

    // create task
    const std::string taskUuid = cluster->addLearnTask(taskData,
                                                       labelData,
                                                       pictureSize,
                                                       10,
                                                       numberOfImages);
    cluster->m_segmentCounter = cluster->getNumberOfSegments();
    cluster->updateClusterState();

    return taskUuid;
}
