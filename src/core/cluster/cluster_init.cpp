/**
 * @file        cluster_init.cpp
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

#include "cluster_init.h"


#include <core/segments/dynamic_segment/dynamic_segment.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>
#include <core/cluster/cluster.h>

#include <core/segments/dynamic_segment/objects.h>
#include <core/segments/dynamic_segment/objects.h>

#include <core/routing_functions.h>
#include <core/cluster/cluster_init.h>

#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCommon/logger.h>

/**
 * @brief re-initialize the pointer in the header of the cluster after restoring the cluster
 *        from a snapshot
 *
 * @param cluster pointer to the cluster
 * @param uuid new uuid of the cluster to override the old one coming from the snapshot
 *
 * @return true, if successful, else false
 */
bool
reinitPointer(Cluster* cluster,
              const std::string &uuid)
{
    uint8_t* dataPtr = static_cast<uint8_t*>(cluster->clusterData.data);
    uint64_t pos = 0;

    // write metadata to buffer
    cluster->networkMetaData = reinterpret_cast<Cluster::MetaData*>(dataPtr + pos);
    pos += sizeof(Cluster::MetaData);

    // write settings to buffer
    cluster->networkSettings = reinterpret_cast<Cluster::Settings*>(dataPtr + pos);
    pos += sizeof(Cluster::Settings);

    // override old uuid with the new one
    strncpy(cluster->networkMetaData->uuid.uuid, uuid.c_str(), 36);
    cluster->networkMetaData->uuid.uuid[36] = '\0';

    cluster->clusterData.usedBufferSize = pos;

    return true;
}

/**
 * @brief init header for a new cluster
 *
 * @param cluster pointer to the cluster where the header belongs to
 * @param metaData metadata-object to write into the header
 * @param settings settings-object to write into the header
 */
void
initHeader(Cluster* cluster,
           const Cluster::MetaData &metaData,
           const Cluster::Settings &settings)
{
    // allocate memory
    const uint32_t numberOfBlocks = 1;
    Kitsunemimi::allocateBlocks_DataBuffer(cluster->clusterData, numberOfBlocks);

    uint8_t* dataPtr = static_cast<uint8_t*>(cluster->clusterData.data);
    uint64_t pos = 0;

    // write metadata to buffer
    cluster->networkMetaData = reinterpret_cast<Cluster::MetaData*>(dataPtr + pos);
    cluster->networkMetaData[0] = metaData;
    pos += sizeof(Cluster::MetaData);

    // write settings to buffer
    cluster->networkSettings = reinterpret_cast<Cluster::Settings*>(dataPtr + pos);
    cluster->networkSettings[0] = settings;
    pos += sizeof(Cluster::Settings);

    cluster->clusterData.usedBufferSize = pos;
}

/**
 * @brief initalize new cluster
 *
 * @param cluster pointer to the uninitionalized cluster
 * @param parsedContent parsed json with the information of the cluster
 * @param segmentTemplates TODO
 * @param uuid uuid for the new cluster
 *
 * @return true, if successful, else false
 */
bool
initNewCluster(Cluster* cluster,
               const JsonItem &parsedContent,
               const std::map<std::string, Kitsunemimi::Json::JsonItem> &segmentTemplates,
               const std::string &uuid)
{
    const JsonItem paredSettings = parsedContent.get("settings");

    // meta-data
    Cluster::MetaData newMetaData;
    strncpy(newMetaData.uuid.uuid, uuid.c_str(), 36);
    newMetaData.uuid.uuid[36] = '\0';

    // settings
    Cluster::Settings newSettings;

    initHeader(cluster, newMetaData, newSettings);

    const std::string name = parsedContent.get("name").getString();
    //const bool ret = cluster->setName(name);  // TODO: handle return

    LOG_INFO("create new cluster with uuid: " + cluster->networkMetaData->uuid.toString());

    // parse and create segments
    for(uint32_t i = 0; i < parsedContent.size(); i++)
    {
        const JsonItem segmentDef = parsedContent.get(i);
        const std::string type = segmentDef.get("type").getString();

        AbstractSegment* newSegment = nullptr;
        if(type == "input")
        {
            newSegment = addInputSegment(cluster, segmentDef);
        }
        else if(type == "output")
        {
            newSegment = addOutputSegment(cluster, segmentDef);
        }
        else
        {
            std::map<std::string, Kitsunemimi::Json::JsonItem>::const_iterator it;
            it = segmentTemplates.find(type);
            if(it != segmentTemplates.end()) {
                newSegment = addDynamicSegment(cluster, segmentDef, it->second);
            }
        }

        if(newSegment == nullptr)
        {
            // TODO: error-handling
            std::cout<<"failed to init segment"<<std::endl;
            return false;
        }

        // update segment information with cluster infos
        newSegment->segmentHeader->parentClusterId = cluster->networkMetaData->uuid;
        newSegment->parentCluster = cluster;
    }

    return true;
}

/**
 * @brief add new input-segment to cluster
 *
 * @param cluster pointer to the uninitionalized cluster
 * @param clusterTemplatePart parsed json with the information of the cluster
 *
 * @return true, if successful, else false
 */
AbstractSegment*
addInputSegment(Cluster* cluster,
                const JsonItem &clusterTemplatePart)
{
    InputSegment* newSegment = new InputSegment();
    JsonItem placeHolder;

    if(newSegment->initSegment(clusterTemplatePart, placeHolder))
    {
        cluster->inputSegments.insert(std::make_pair(newSegment->getName(), newSegment));
        cluster->allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief add new output-segment to cluster
 *
 * @param cluster pointer to the uninitionalized cluster
 * @param clusterTemplatePart parsed json with the information of the cluster
 *
 * @return true, if successful, else false
 */
AbstractSegment*
addOutputSegment(Cluster* cluster,
                 const JsonItem &clusterTemplatePart)
{
    OutputSegment* newSegment = new OutputSegment();
    JsonItem placeHolder;

    if(newSegment->initSegment(clusterTemplatePart, placeHolder))
    {
        cluster->outputSegments.insert(std::make_pair(newSegment->getName(), newSegment));
        cluster->allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief add new dynamic-segment to cluster
 *
 * @param cluster pointer to the uninitionalized cluster
 * @param clusterTemplatePart parsed json with the information of the cluster
 *
 * @return true, if successful, else false
 */
AbstractSegment*
addDynamicSegment(Cluster* cluster,
                  const JsonItem &clusterTemplatePart,
                  const JsonItem &segmentTemplate)
{
    DynamicSegment* newSegment = new DynamicSegment();

    if(newSegment->initSegment(clusterTemplatePart, segmentTemplate))
    {
        cluster->allSegments.push_back(newSegment);
    }
    else
    {
        delete newSegment;
        newSegment = nullptr;
    }

    return newSegment;
}

/**
 * @brief convert position information from json to position-item
 *
 * @param parsedContent json of the segment
 *
 * @return postion-objectfor the segment
 */
Position
convertPosition(const JsonItem &parsedContent)
{
    const JsonItem parsedPosition = parsedContent.get("position");

    Position currentPosition;
    currentPosition.x = parsedPosition.get(0).getInt();
    currentPosition.y = parsedPosition.get(1).getInt();
    currentPosition.z = parsedPosition.get(2).getInt();

    return currentPosition;
}
