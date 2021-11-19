/**
 * @file        learn_blossom.cpp
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

#include "learn_blossom.h"

#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>

#include <libKitsunemimiCrypto/common.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

LearnBlossom::LearnBlossom()
    : Blossom()
{
    registerInputField("cluster_uuid", true);
    registerInputField("inputs", true);
    registerInputField("number_of_inputs_per_cycle", true);
    registerInputField("number_of_outputs_per_cycle", true);
    registerInputField("number_of_cycles", true);

    registerOutputField("task_uuid", true);
}

/**
 * @brief LearnBlossom::runTask
 * @param blossomLeaf
 * @param errorMessage
 * @return
 */
bool
LearnBlossom::runTask(BlossomLeaf &blossomLeaf,
                      BlossomStatus &status,
                      Kitsunemimi::ErrorContainer &error)
{
    // get id
    const std::string uuid = blossomLeaf.input.getStringByKey("cluster_uuid");
    ClusterInterface* interface = KyoukoRoot::m_root->m_clusterHandler->getCluster(uuid);
    if(interface == nullptr)
    {
        error.addMeesage("interface with uuid not found: " + uuid);
        return false;
    }

    // get sizes
    const uint32_t inputsPerCycle = blossomLeaf.input.getIntByKey("number_of_inputs_per_cycle");
    const uint32_t outputsPerCycle = blossomLeaf.input.getIntByKey("number_of_outputs_per_cycle");
    const uint32_t numberOfCycles = blossomLeaf.input.getIntByKey("number_of_cycles");

    // get input-data
    const std::string inputs = blossomLeaf.input.getStringByKey("inputs");
    DataBuffer resultBuffer;
    const bool ret = Kitsunemimi::Crypto::decodeBase64(resultBuffer, inputs);
    if(ret == false)
    {
        error.addMeesage("base64-decoding of the input failes");
        return false;
    }

    const std::string taskUuid = interface->addLearnTask((float*)resultBuffer.data,
                                                         inputsPerCycle,
                                                         outputsPerCycle,
                                                         numberOfCycles);
    blossomLeaf.output.insert("task_uuid", new DataValue(taskUuid));

    return true;
}
