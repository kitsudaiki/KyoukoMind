/**
 * @file        ask_blossom.cpp
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

#include "ask_blossom.h"
#include <kyouko_root.h>

#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>

#include <libKitsunemimiCrypto/common.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi::Sakura;

AskBlossom::AskBlossom()
    : Blossom("Request information from the network.")
{
    registerInputField("cluster_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster, which should process the request");
    registerInputField("inputs",
                       SAKURA_STRING_TYPE,
                       true,
                       "Input-data as base64 encoded string.");
    registerInputField("number_of_inputs_per_cycle",
                       SAKURA_INT_TYPE,
                       true,
                       "Number of inputs per set.");
    registerInputField("number_of_cycles",
                       SAKURA_INT_TYPE,
                       true,
                       "Number of sets.");

    registerOutputField("task_uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created task.");
}

/**
 * @brief AskBlossom::runTask
 * @param blossomLeaf
 * @param errorMessage
 * @return
 */
bool
AskBlossom::runTask(BlossomLeaf &blossomLeaf,
                    const Kitsunemimi::DataMap &,
                    BlossomStatus &status,
                    Kitsunemimi::ErrorContainer &error)
{
    const uint32_t inputsPerCycle = blossomLeaf.input.getIntByKey("number_of_inputs_per_cycle");
    const uint32_t numberOfCycles = blossomLeaf.input.getIntByKey("number_of_cycles");
    const std::string uuid = blossomLeaf.input.getStringByKey("cluster_uuid");
    const std::string inputs = blossomLeaf.input.getStringByKey("inputs");

    ClusterInterface* cluster = KyoukoRoot::m_clusterHandler->getCluster(uuid);
    if(cluster == nullptr)
    {
        status.errorMessage = "cluster with uuid '" + uuid + "'not found";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        error.addMeesage(status.errorMessage);
        return false;
    }

    // get input-data
    DataBuffer resultBuffer;
    if(Kitsunemimi::Crypto::decodeBase64(resultBuffer, inputs) == false)
    {
        error.addMeesage("base64-decoding of the input failes");
        return false;
    }

    const std::string taskUuid = cluster->addRequestTask((float*)resultBuffer.data,
                                                         inputsPerCycle,
                                                         numberOfCycles);
    resultBuffer.data = nullptr;

    blossomLeaf.output.insert("task_uuid", new DataValue(taskUuid));

    return true;
}
