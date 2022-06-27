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

#include <libSagiriArchive/snapshots.h>

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
        Task* actualTask = m_cluster->getActualTask();
        uint64_t totalSize = 0;
        std::string headerMessage = "";

        // create message to sagiri and calculate total size of storage of the cluster
        totalSize = m_cluster->clusterData.usedBufferSize;
        headerMessage = "{\"header\":" + std::to_string(totalSize) + ",\"segments\":[";
        for(uint64_t i = 0; i < m_cluster->allSegments.size(); i++)
        {
            if(i != 0) {
                headerMessage += ",";
            }
            const uint64_t segSize = m_cluster->allSegments.at(i)->segmentData.buffer.usedBufferSize;
            headerMessage += "{\"size\":"
                               + std::to_string(segSize)
                               + ",\"type\":"
                               + std::to_string(m_cluster->allSegments.at(i)->getType())
                               + "}";
            totalSize += segSize;
        }
        headerMessage += "]}";

        // send snapshot to sagiri
        std::string fileUuid = "";
        if(Sagiri::runInitProcess(fileUuid,
                                  actualTask->uuid.toString(),
                                  actualTask->metaData.getStringByKey("snapshot_name"),
                                  actualTask->metaData.getStringByKey("user_uuid"),
                                  actualTask->metaData.getStringByKey("project_uuid"),
                                  totalSize,
                                  headerMessage,
                                  *KyoukoRoot::componentToken,
                                  error) == false)
        {
            error.addMeesage("Failed to run initializing a snapshot-transfer to sagiri");
            break;
        }

        if(sendData(actualTask->uuid.toString(),
                    fileUuid,
                    error) == false)
        {
            error.addMeesage("Failed to send data of snapshot to sagiri");
            break;
        }

        if(Sagiri::runFinalizeProcess(actualTask->uuid.toString(),
                                      fileUuid,
                                      *KyoukoRoot::componentToken,
                                      actualTask->metaData.getStringByKey("user_uuid"),
                                      actualTask->metaData.getStringByKey("project_uuid"),
                                      error) == false)
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
 * @brief send all data of the snapshot to sagiri
 *
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
SaveCluster_State::sendData(const std::string &snapshotUuid,
                            const std::string &fileUuid,
                            Kitsunemimi::ErrorContainer &error)
{
    // global byte-counter to identifiy the position within the complete snapshot
    uint64_t posCounter = 0;

    // send cluster-metadata
    if(Sagiri::sendData(&m_cluster->clusterData,
                        posCounter,
                        snapshotUuid,
                        fileUuid,
                        error) == false)
    {
        error.addMeesage("Failed to send metadata of cluster for snapshot to sagiri");
        return false;
    }

    // send segments of cluster
    for(uint64_t i = 0; i < m_cluster->allSegments.size(); i++)
    {
        if(Sagiri::sendData(&m_cluster->allSegments.at(i)->segmentData.buffer,
                            posCounter,
                            snapshotUuid,
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

