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

#include "create_image_request_task.h"
#include <kyouko_root.h>

#include <libShioriArchive/datasets.h>

#include <core/cluster/cluster_handler.h>
#include <core/cluster/cluster.h>

#include <libKitsunemimiHanamiCommon/component_support.h>
#include <libKitsunemimiHanamiCommon/enums.h>
#include <libKitsunemimiHanamiNetwork/hanami_messaging.h>

#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Hanami;
using Kitsunemimi::Hanami::SupportedComponents;

CreateImageRequestTask::CreateImageRequestTask()
    : Blossom("Add new image-request-task to the task-queue of a cluster.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name for the new task for better identification.");
    assert(addFieldBorder("name", 4, 256));
    assert(addFieldRegex("name", NAME_REGEX));

    registerInputField("cluster_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster, which should process the request");
    assert(addFieldRegex("cluster_uuid", UUID_REGEX));

    registerInputField("data_set_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the data-set with the input, which coming from shiori.");
    assert(addFieldRegex("data_set_uuid", UUID_REGEX));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created task.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new created task.");


    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
CreateImageRequestTask::runTask(BlossomIO &blossomIO,
                                const Kitsunemimi::DataMap &context,
                                BlossomStatus &status,
                                Kitsunemimi::ErrorContainer &error)
{
    const std::string name = blossomIO.input.get("name").getString();
    const std::string clusterUuid = blossomIO.input.get("cluster_uuid").getString();
    const std::string dataSetUuid = blossomIO.input.get("data_set_uuid").getString();
    const Kitsunemimi::Hanami::UserContext userContext(context);

    SupportedComponents* scomp = SupportedComponents::getInstance();
    if(scomp->support[Kitsunemimi::Hanami::SHIORI] == false)
    {
        status.statusCode = Kitsunemimi::Hanami::SERVICE_UNAVAILABLE_RTYPE;
        status.errorMessage = "Shiori is not configured for Kyouko.";
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

    // get meta-infos of data-set from shiori
    JsonItem dataSetInfo;
    if(Shiori::getDataSetInformation(dataSetInfo, dataSetUuid, userContext.token, error) == false)
    {
        error.addMeesage("Failed to get information from shiori for UUID '" + dataSetUuid + "'");
        // TODO: add status-error from response from shiori
        status.statusCode = Kitsunemimi::Hanami::UNAUTHORIZED_RTYPE;
        return false;
    }

    // get relevant information from output
    const uint64_t numberOfInputs = dataSetInfo.get("inputs").getLong();
    const uint64_t numberOfOutputs = dataSetInfo.get("outputs").getLong();
    const uint64_t numberOfLines = dataSetInfo.get("lines").getLong();

    // get input-data
    DataBuffer* dataSetBuffer = Shiori::getDatasetData(userContext.token, dataSetUuid, "", error);
    if(dataSetBuffer == nullptr)
    {
        error.addMeesage("Failed to get data from shiori for dataset with UUID '"
                         + dataSetUuid
                         + "'");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // init request-task
    // TODO: fix line limitation
    float* floatData = static_cast<float*>(dataSetBuffer->data);
    const std::string taskUuid = cluster->addImageRequestTask(name,
                                                              userContext.userId,
                                                              userContext.projectId,
                                                              floatData,
                                                              numberOfInputs,
                                                              numberOfOutputs,
                                                              numberOfLines);

    blossomIO.output.insert("uuid", taskUuid);
    blossomIO.output.insert("name", name);

    return true;
}
