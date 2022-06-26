/**
 * @file        save_cluster_state.cpp
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

#include "save_cluster_state.h"

#include <core/cluster/task.h>
#include <core/cluster/cluster.h>
#include <core/cluster/statemachine_init.h>
#include <core/segments/abstract_segment.h>

#include <libKitsunemimiHanamiMessaging/hanami_messaging_client.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

#include <libKitsunemimiCrypto/hashes.h>

using Kitsunemimi::Hanami::HanamiMessaging;
using Kitsunemimi::Hanami::HanamiMessagingClient;

/**
 * @brief constructor
 *
 * @param cluster pointer to the cluster, where the event and the statemachine belongs to
 */
SaveCluster_State::SaveCluster_State(Cluster* cluster)
{
    m_cluster = cluster;
}

/**
 * @brief destructor
 */
SaveCluster_State::~SaveCluster_State() {}

/**
 * @brief prcess event
 *
 * @return true, if successful, else false
 */
bool
SaveCluster_State::processEvent()
{
    bool result = false;

    Kitsunemimi::ErrorContainer error;

    do
    {
        // get internal client for interaction with sagiri
        HanamiMessaging* messaging = HanamiMessaging::getInstance();
        m_client = messaging->sagiriClient;
        if(m_client == nullptr)
        {
            error.addMeesage("Failed to get client to sagiri");
            error.addSolution("Check if sagiri is correctly configured");
            break;
        }

        // send snapshot to sagiri
        if(runInitProcess(error) == false)
        {
            error.addMeesage("Failed to run initializing a snapshot-transfer to sagiri");
            break;
        }
        if(sendData(error) == false)
        {
            error.addMeesage("Failed to send data of snapshot to sagiri");
            break;
        }
        if(runFinalizeProcess(error) == false)
        {
            error.addMeesage("Failed to run finalizing a snapshot-transfer to sagiri");
            break;
        }

        result = true;
        break;
    }
    while(true);

    m_cluster->goToNextState(FINISH_TASK);

    if(result == false)
    {
        error.addMeesage("Failed to create snapshot of cluster with UUID '"
                         + m_cluster->getUuid()
                         + "'");
        // TODO: give the user a feedback by setting the task to failed-state
    }

    return result;
}

/**
 * @brief initialize the transfer of the cluster-snapshot to sagiri
 *
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
SaveCluster_State::runInitProcess(Kitsunemimi::ErrorContainer &error)
{
    Task* actualTask = m_cluster->getActualTask();

    // create message to sagiri and calculate total size of storage of the cluster
    m_totalSize = m_cluster->clusterData.usedBufferSize;
    m_headerMessage = "{\"header\":" + std::to_string(m_totalSize) + ",\"segments\":[";
    for(uint64_t i = 0; i < m_cluster->allSegments.size(); i++)
    {
        if(i != 0) {
            m_headerMessage += ",";
        }
        const uint64_t segSize = m_cluster->allSegments.at(i)->segmentData.buffer.usedBufferSize;
        m_headerMessage += "{\"size\":"
                           + std::to_string(segSize)
                           + ",\"type\":"
                           + std::to_string(m_cluster->allSegments.at(i)->getType())
                           + "}";
        m_totalSize += segSize;
    }
    m_headerMessage += "]}";

    // create request
    Kitsunemimi::Hanami::RequestMessage requestMsg;
    requestMsg.id = "v1/cluster_snapshot";
    requestMsg.httpType = Kitsunemimi::Hanami::HttpRequestType::POST_TYPE;
    requestMsg.inputValues = "";
    requestMsg.inputValues.append("{\"user_uuid\":\"");
    requestMsg.inputValues.append(actualTask->metaData.getStringByKey("user_uuid"));
    requestMsg.inputValues.append("\",\"token\":\"");
    requestMsg.inputValues.append(*KyoukoRoot::componentToken);
    requestMsg.inputValues.append("\",\"uuid\":\"");
    requestMsg.inputValues.append(actualTask->uuid.toString());
    requestMsg.inputValues.append("\",\"header\":");
    requestMsg.inputValues.append(m_headerMessage);
    requestMsg.inputValues.append(",\"project_uuid\":\"");
    requestMsg.inputValues.append(actualTask->metaData.getStringByKey("project_uuid"));
    requestMsg.inputValues.append("\",\"name\":\"");
    requestMsg.inputValues.append(actualTask->metaData.getStringByKey("snapshot_name"));
    requestMsg.inputValues.append("\",\"input_data_size\":");
    requestMsg.inputValues.append(std::to_string(m_totalSize));
    requestMsg.inputValues.append("}");

    // trigger initializing of snapshot
    Kitsunemimi::Hanami::ResponseMessage response;
    if(m_client->triggerSakuraFile(response, requestMsg, error) == false)
    {
        error.addMeesage("Failed to trigger blossom in sagiri to initialize "
                         "the transfer of a cluster");
        return false;
    }

    // check response
    if(response.success == false)
    {
        error.addMeesage(response.responseContent);
        error.addMeesage("Failed to trigger blossom in sagiri to initialize "
                         "the transfer of a cluster");
        return false;
    }

    // process response
    LOG_DEBUG("Response from initializing cluster-snapshot: " + response.responseContent);
    if(m_parsedResponse.parse(response.responseContent, error) == false)
    {
        error.addMeesage("Failed to parse reponse from sagiri for the initializing "
                         "of the snapshot-transfer");
        return false;
    }

    return true;
}

/**
 * @brief send all data of the snapshot to sagiri
 *
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
SaveCluster_State::sendData(Kitsunemimi::ErrorContainer &error)
{
    const std::string uuid = m_parsedResponse.get("uuid").getString();
    const std::string fileUuid = m_parsedResponse.get("uuid_input_file").getString();

    // global byte-counter to identifiy the position within the complete snapshot
    uint64_t posCounter = 0;

    // send cluster-metadata
    if(sendData(&m_cluster->clusterData, posCounter, uuid, fileUuid, error) == false)
    {
        error.addMeesage("Failed to send metadata of cluster for snapshot to sagiri");
        return false;
    }

    // send segments of cluster
    for(uint64_t i = 0; i < m_cluster->allSegments.size(); i++)
    {
        if(sendData(&m_cluster->allSegments.at(i)->segmentData.buffer,
                    posCounter,
                    uuid,
                    fileUuid,
                    error) == false)
        {
            error.addMeesage("Failed to send snapshot of segment '"
                             + std::to_string(i)
                             + "' to sagiri");
            return false;
        }
    }

    return true;
}

/**
 * @brief send data of the snapshot to sagiri
 *
 * @param data buffer with data to send
 * @param targetPos byte-position within the snapshot where the data belongs to
 * @param uuid uuid of the snapshot
 * @param fileUuid uuid of the temporary file in sagiri for identification
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
SaveCluster_State::sendData(const Kitsunemimi::DataBuffer* data,
                            uint64_t &targetPos,
                            const std::string &uuid,
                            const std::string &fileUuid,
                            Kitsunemimi::ErrorContainer &error)
{
    const uint64_t dataSize = data->usedBufferSize;
    const uint8_t* u8Data = static_cast<const uint8_t*>(data->data);

    uint8_t sendBuffer[128*1024];
    uint64_t i = 0;
    uint64_t pos = 0;

    // prepare buffer
    uint64_t segmentSize = 96 * 1024;
    memcpy(&sendBuffer[0], uuid.c_str(), 36);
    memcpy(&sendBuffer[36], fileUuid.c_str(), 36);

    do
    {
        pos = i + targetPos;
        memcpy(&sendBuffer[72], &pos, 8);

        // check the size for the last segment
        segmentSize = 96 * 1024;
        if(dataSize - i < segmentSize) {
            segmentSize = dataSize - i;
        }

        // read segment of the local file
        memcpy(&sendBuffer[80], &u8Data[i], segmentSize);

        // send segment
        if(m_client->sendStreamMessage(sendBuffer, segmentSize + 80, false, error) == false)
        {
            error.addMeesage("Failed to send part with position '"
                             + std::to_string(i)
                             + "' to sagiri");
            return false;
        }

        i += segmentSize;
    }
    while(i < dataSize);

    targetPos += i;

    return true;
}

/**
 * @brief finalize the transfer of the snapshot to sagiri
 *
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
SaveCluster_State::runFinalizeProcess(Kitsunemimi::ErrorContainer &error)
{
    Task* actualTask = m_cluster->getActualTask();
    const std::string uuid = m_parsedResponse.get("uuid").getString();
    const std::string fileUuid = m_parsedResponse.get("uuid_input_file").getString();

    // create request
    Kitsunemimi::Hanami::RequestMessage requestMsg;
    requestMsg.id = "v1/cluster_snapshot";
    requestMsg.httpType = Kitsunemimi::Hanami::HttpRequestType::PUT_TYPE;
    requestMsg.inputValues = "";
    requestMsg.inputValues.append("{\"user_uuid\":\"");
    requestMsg.inputValues.append(actualTask->metaData.getStringByKey("user_uuid"));
    requestMsg.inputValues.append("\",\"token\":\"");
    requestMsg.inputValues.append(*KyoukoRoot::componentToken);
    requestMsg.inputValues.append("\",\"project_uuid\":\"");
    requestMsg.inputValues.append(actualTask->metaData.getStringByKey("project_uuid"));
    requestMsg.inputValues.append("\",\"uuid\":\"");
    requestMsg.inputValues.append(uuid);
    requestMsg.inputValues.append("\",\"uuid_input_file\":\"");
    requestMsg.inputValues.append(fileUuid);
    requestMsg.inputValues.append("\"}");

    // trigger finalizing of snapshot
    Kitsunemimi::Hanami::ResponseMessage response;
    if(m_client->triggerSakuraFile(response, requestMsg, error) == false)
    {
        error.addMeesage("Failed to trigger blossom in sagiri to finalize "
                         "the transfer of a cluster");
        return false;
    }

    // check response
    if(response.success == false)
    {
        error.addMeesage(response.responseContent);
        error.addMeesage("Failed to trigger blossom in sagiri to finalize "
                         "the transfer of a cluster");
        return false;
    }

    // process response
    LOG_DEBUG("Response from finalizing cluster-snapshot: " + response.responseContent);
    if(m_parsedResponse.parse(response.responseContent, error) == false)
    {
        error.addMeesage("Failed to parse reponse from sagiri for the finalizeing "
                         "of the snapshot-transfer");
        return false;
    }

    return true;
}
