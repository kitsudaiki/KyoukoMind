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
#include <core/segments/abstract_segment.h>

#include <libKitsunemimiHanamiMessaging/hanami_messaging_client.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging.h>

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
 * @return alway true
 */
bool
SaveCluster_State::processEvent()
{
    HanamiMessaging* messaging = HanamiMessaging::getInstance();
    m_client = messaging->sagiriClient;
    if(m_client == nullptr)
    {
        // TODO: error-message
        return false;
    }

    if(handleInitProcess() == false) {
        return false;
    }
    if(sendData() == false) {
        return false;
    }
    if(handleFinalizeProcess() == false) {
        return false;
    }

    return true;
}

/**
 * @brief SaveCluster_State::handleInitProcess
 * @return
 */
bool
SaveCluster_State::handleInitProcess()
{
    Task* actualTask = m_cluster->getActualTask();

    // get total size of memory of the cluster
    m_totalSize = m_cluster->clusterData.usedBufferSize;
    for(uint64_t i = 0; i < m_cluster->allSegments.size(); i++) {
        m_totalSize += m_cluster->allSegments.at(i)->segmentData.buffer.usedBufferSize;
    }

    // create request
    Kitsunemimi::Hanami::RequestMessage requestMsg;
    requestMsg.id = "v1/cluster_snapshot";
    requestMsg.httpType = Kitsunemimi::Hanami::HttpRequestType::POST_TYPE;
    requestMsg.inputValues = "";
    requestMsg.inputValues.append("{\"user_uuid\":\"");
    requestMsg.inputValues.append(actualTask->metaData.getStringByKey("user_uuid"));
    requestMsg.inputValues.append("\",\"token\":\"");
    requestMsg.inputValues.append(*KyoukoRoot::componentToken);
    requestMsg.inputValues.append("\",\"project_uuid\":\"");
    requestMsg.inputValues.append(actualTask->metaData.getStringByKey("project_uuid"));
    requestMsg.inputValues.append("\",\"name\":\"");
    requestMsg.inputValues.append(actualTask->metaData.getStringByKey("snapshot_name"));
    requestMsg.inputValues.append("\",\"input_data_size\":");
    requestMsg.inputValues.append(std::to_string(m_totalSize));
    requestMsg.inputValues.append("}");

    // make token-request
    Kitsunemimi::Hanami::ResponseMessage response;
    Kitsunemimi::ErrorContainer error;
    if(m_client->triggerSakuraFile(response, requestMsg, error) == false)
    {
        // TODO: error-message
        return false;
    }

    if(response.success == false)
    {
        // TODO: error-message
        return false;
    }

    LOG_DEBUG("Response from initializing cluster-snapshot: " + response.responseContent);
    if(m_parsedResponse.parse(response.responseContent, error) == false)
    {
        // TODO: error-message
        return false;
    }

    return true;
}

/**
 * @brief SaveCluster_State::sendData
 * @return
 */
bool
SaveCluster_State::sendData()
{
    const std::string uuid = m_parsedResponse.get("uuid").getString();
    const std::string fileUuid = m_parsedResponse.get("uuid_input_file").getString();
    uint64_t posCounter = 0;

    // send cluster-header
    if(sendData(&m_cluster->clusterData, posCounter, uuid, fileUuid) == false) {
        return false;
    }

    // send segments of cluster
    for(uint64_t i = 0; i < m_cluster->allSegments.size(); i++)
    {
        if(sendData(&m_cluster->allSegments.at(i)->segmentData.buffer,
                    posCounter,
                    uuid,
                    fileUuid) == false)
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief SaveCluster_State::sendData
 * @param data
 * @param targetPos
 * @return
 */
bool
SaveCluster_State::sendData(const Kitsunemimi::DataBuffer* data,
                            uint64_t &targetPos,
                            const std::string &uuid,
                            const std::string &fileUuid)
{
    const uint64_t dataSize = data->usedBufferSize;
    const uint8_t* u8Data = static_cast<const uint8_t*>(data->data);

    uint8_t sendBuffer[128*1024];
    Kitsunemimi::ErrorContainer error;
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
        memcpy(&sendBuffer[80], u8Data, segmentSize);

        // send segment
        if(m_client->sendStreamMessage(sendBuffer, segmentSize + 80, false, error) == false) {
            return false;
        }

        i += segmentSize;
    }
    while(i < dataSize);

    targetPos += i;

    return true;
}

/**
 * @brief SaveCluster_State::handleFinalizeProcess
 * @return
 */
bool
SaveCluster_State::handleFinalizeProcess()
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

    // make token-request
    Kitsunemimi::Hanami::ResponseMessage response;
    Kitsunemimi::ErrorContainer error;
    if(m_client->triggerSakuraFile(response, requestMsg, error) == false)
    {
        // TODO: error-message
        return false;
    }

    if(response.success == false)
    {
        // TODO: error-message
        return false;
    }

    LOG_DEBUG("Response from finalizing cluster-snapshot: " + response.responseContent);
    if(m_parsedResponse.parse(response.responseContent, error) == false)
    {
        // TODO: error-message
        return false;
    }

    return true;
}
