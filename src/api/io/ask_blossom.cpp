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

#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>

#include <libKitsunemimiCrypto/common.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

AskBlossom::AskBlossom()
    : Blossom()
{
    registerField("cluster_uuid", INPUT_TYPE, true);
    registerField("inputs", INPUT_TYPE, true);
    registerField("number_of_inputs_per_cycle", INPUT_TYPE, true);
    registerField("number_of_cycles", INPUT_TYPE, true);
    registerField("task_uuid", OUTPUT_TYPE, true);
}

/**
 * @brief AskBlossom::runTask
 * @param blossomLeaf
 * @param errorMessage
 * @return
 */
bool
AskBlossom::runTask(BlossomLeaf &blossomLeaf,
                    std::string &errorMessage)
{
    // get id
    const std::string uuid = blossomLeaf.input.getStringByKey("cluster_uuid");
    ClusterInterface* interface = KyoukoRoot::m_root->m_clusterHandler->getCluster(uuid);
    if(interface == nullptr)
    {
        errorMessage = "interface with uuid not found: " + uuid;
        return false;
    }

    // get sizes
    const uint32_t inputsPerCycle = blossomLeaf.input.getIntByKey("number_of_inputs_per_cycle");
    const uint32_t numberOfCycles = blossomLeaf.input.getIntByKey("number_of_cycles");

    // get input-data
    const std::string inputs = blossomLeaf.input.getStringByKey("inputs");
    DataBuffer resultBuffer;
    const bool ret = Kitsunemimi::Crypto::decodeBase64(resultBuffer, inputs);
    if(ret == false)
    {
        errorMessage = "base64-decoding of the input failes";
        return false;
    }

    const std::string taskUuid = interface->addRequestTask((float*)resultBuffer.data,
                                                           inputsPerCycle,
                                                           numberOfCycles);
    blossomLeaf.output.insert("task_uuid", new DataValue(taskUuid));

    return true;
}
