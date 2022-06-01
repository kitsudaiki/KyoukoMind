/**
 * @file        restore_cluster_state.cpp
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

#include "restore_cluster_state.h"
#include <kyouko_root.h>
#include <core/cluster/task.h>
#include <core/cluster/cluster.h>
#include <core/cluster/cluster_init.h>
#include <core/segments/abstract_segment.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>
#include <core/segments/dynamic_segment/dynamic_segment.h>

#include <libSagiriArchive/sagiri_send.h>

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
RestoreCluster_State::RestoreCluster_State(Cluster* cluster)
{
    m_cluster = cluster;
}

/**
 * @brief destructor
 */
RestoreCluster_State::~RestoreCluster_State() {}

/**
 * @brief prcess event
 *
 * @return alway true
 */
bool
RestoreCluster_State::processEvent()
{
    Task* actualTask = m_cluster->getActualTask();
    Kitsunemimi::ErrorContainer error;
    HanamiMessaging* messaging = HanamiMessaging::getInstance();
    m_client = messaging->sagiriClient;
    if(m_client == nullptr)
    {
        // TODO: error-message
        m_cluster->goToNextState(Cluster::FINISH_TASK);
        return false;
    }

    const std::string snapshotInfo = actualTask->metaData.get("snapshot_info")->getString();
    const std::string userUuid = actualTask->metaData.get("user_uuid")->getString();
    const std::string projectUuid = actualTask->metaData.get("project_uuid")->getString();

    // get meta-infos of data-set from sagiri
    Kitsunemimi::Json::JsonItem parsedSnapshotInfo;
    parsedSnapshotInfo.parse(snapshotInfo, error);
    const std::string snapshotUuid = parsedSnapshotInfo.get("uuid").getString();
    const std::string header = parsedSnapshotInfo.get("header").toString();
    const std::string location = parsedSnapshotInfo.get("location").toString();

    Kitsunemimi::Json::JsonItem parsedHeader;
    if(parsedHeader.parse(header, error) == false)
    {
        m_cluster->goToNextState(Cluster::FINISH_TASK);
        return false;
    }

    // get snapshot-data
    DataBuffer* snapshotBuffer = Sagiri::getSnapshotData(location, error);
    if(snapshotBuffer == nullptr)
    {
        error.addMeesage("failed to get snapshot-data from sagiri");
        m_cluster->goToNextState(Cluster::FINISH_TASK);

        return false;
    }

    for(uint64_t i = 0; i < m_cluster->allSegments.size(); i++)
    {
        AbstractSegment* segment = m_cluster->allSegments.at(i);
        delete segment;
        m_cluster->allSegments.clear();
        m_cluster->inputSegments.clear();
        m_cluster->outputSegments.clear();
    }

    const uint8_t* u8Data = static_cast<const uint8_t*>(snapshotBuffer->data);

    // copy meta-data of cluster
    const uint64_t headerSize = parsedHeader.get("header").getLong();
    if(Kitsunemimi::reset_DataBuffer(m_cluster->clusterData,
                                     Kitsunemimi::calcBytesToBlocks(headerSize)) == false)
    {
        // TODO: handle error
        return false;
    }
    memcpy(m_cluster->clusterData.data, &u8Data[0], headerSize);
    reinitPointer(m_cluster, snapshotUuid);

    // copy single segments
    uint64_t posCounter = headerSize;
    for(uint64_t i = 0; i < parsedHeader.get("segments").size(); i++)
    {
        JsonItem segment = parsedHeader.get("segments").get(i);
        const SegmentTypes type = static_cast<SegmentTypes>(segment.get("type").getInt());
        const uint64_t size = static_cast<uint64_t>(segment.get("size").getLong());

        switch(type)
        {
            case INPUT_SEGMENT:
            {
                InputSegment* newSegment = new InputSegment(&u8Data[posCounter], size);
                newSegment->reinitPointer(size);
                newSegment->parentCluster = m_cluster;
                m_cluster->inputSegments.push_back(newSegment);
                m_cluster->allSegments.push_back(newSegment);
                break;
            }
            case OUTPUT_SEGMENT:
            {
                OutputSegment* newSegment = new OutputSegment(&u8Data[posCounter], size);
                newSegment->reinitPointer(size);
                newSegment->parentCluster = m_cluster;
                m_cluster->outputSegments.push_back(newSegment);
                m_cluster->allSegments.push_back(newSegment);
                break;
            }
            case DYNAMIC_SEGMENT:
            {
                DynamicSegment* newSegment = new DynamicSegment(&u8Data[posCounter], size);
                newSegment->reinitPointer(size);
                newSegment->parentCluster = m_cluster;
                m_cluster->allSegments.push_back(newSegment);
                break;
            }
            case UNDEFINED_SEGMENT:
            {
                break;
            }
        }

        posCounter += size;
    }

    delete snapshotBuffer;

    m_cluster->goToNextState(Cluster::FINISH_TASK);

    return true;
}
