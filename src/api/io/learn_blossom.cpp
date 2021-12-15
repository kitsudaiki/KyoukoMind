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
    : Blossom("Learn a new set of data.")
{
    registerInputField("cluster_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster, which should process the request");
    registerInputField("inputs",
                       SAKURA_STRING_TYPE,
                       true,
                       "Input-data as base64 encoded string.");
    registerInputField("label",
                       SAKURA_STRING_TYPE,
                       true,
                       "List with the labels for the input-data as base64 encoded string.");
    registerInputField("number_of_inputs_per_cycle",
                       SAKURA_INT_TYPE,
                       true,
                       "Number of input-data per input-set.");
    registerInputField("number_of_outputs_per_cycle",
                       SAKURA_INT_TYPE,
                       true,
                       "Number of labels-data per input-set");
    registerInputField("Number_of_cycles",
                       SAKURA_INT_TYPE,
                       true,
                       "Total number of sets.");

    registerOutputField("task_uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created task.");
}

/**
 * @brief LearnBlossom::runTask
 * @param blossomLeaf
 * @param errorMessage
 * @return
 */
bool
LearnBlossom::runTask(BlossomLeaf &blossomLeaf,
                      const Kitsunemimi::DataMap &,
                      BlossomStatus &,
                      Kitsunemimi::ErrorContainer &error)
{
    const uint32_t inputsPerCycle = blossomLeaf.input.get("number_of_inputs_per_cycle").getInt();
    const uint32_t outputsPerCycle = blossomLeaf.input.get("number_of_outputs_per_cycle").getInt();
    const uint32_t numberOfCycles = blossomLeaf.input.get("number_of_cycles").getInt();
    const std::string uuid = blossomLeaf.input.get("cluster_uuid").getString();

    // get cluster
    ClusterInterface* cluster = KyoukoRoot::m_clusterHandler->getCluster(uuid);
    if(cluster == nullptr)
    {
        error.addMeesage("interface with uuid not found: " + uuid);
        return false;
    }

    // get input-data
    const std::string inputs = blossomLeaf.input.get("inputs").getString();
    DataBuffer inputBuffer;
    if(Kitsunemimi::Crypto::decodeBase64(inputBuffer, inputs) == false)
    {
        error.addMeesage("base64-decoding of the input failes");
        return false;
    }

    // get label-data
    const std::string label = blossomLeaf.input.get("label").getString();
    DataBuffer labelBuffer;
    if(Kitsunemimi::Crypto::decodeBase64(labelBuffer, label) == false)
    {
        error.addMeesage("base64-decoding of the input failes");
        return false;
    }

    // init learn-task
    const std::string taskUuid = cluster->addLearnTask((float*)inputBuffer.data,
                                                       (float*)labelBuffer.data,
                                                       inputsPerCycle,
                                                       outputsPerCycle,
                                                       numberOfCycles);
    inputBuffer.data = nullptr;
    labelBuffer.data = nullptr;

    blossomLeaf.output.insert("task_uuid", taskUuid);

    return true;
}
