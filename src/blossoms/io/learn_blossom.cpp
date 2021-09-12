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

#include <libKitsunemimiJson/json_item.h>
#include <core/processing/cpu/cpu_processing_unit.h>
#include <core/objects/network_cluster.h>
#include <core/objects/segments/input_segment.h>
#include <core/objects/segments/output_segment.h>
#include <core/cluster_handler.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;
using namespace Kitsunemimi::Json;

LearnBlossom::LearnBlossom()
    : Blossom()
{
    registerField("request", INPUT_TYPE, true);
}

bool
LearnBlossom::runTask(BlossomLeaf &blossomLeaf,
                      std::string &errorMessage)
{
    const std::string uuid = blossomLeaf.input.getStringByKey("cluster_uuid");
    NetworkCluster* cluster = KyoukoRoot::m_root->m_clusterHandler->getCluster(uuid);
    // TODO: handle if not found

    InputNode* inputNodes = cluster->inputSegments[0]->inputs;
    OutputNode* outputs = cluster->outputSegments[0]->outputs;
    CpuProcessingUnit cpuProcessingUnit;

    const std::string requestString = blossomLeaf.input.getStringByKey("request");
    JsonItem request;
    if(request.parse(requestString, errorMessage) == false) {
        return false;
    }

    const uint32_t numberOfInputs = request["number_of_inputs"].getInt();
    const float reduction = request["reduction"].getFloat();

    for(uint32_t pic = 0; pic < numberOfInputs; pic++)
    {
        JsonItem input = request["input"][pic];
        JsonItem should = request["should"][pic];

        for(uint32_t i = 0; i < should.size(); i++) {
            outputs[i].shouldValue = should[i].getFloat();
        }

        for(uint32_t i = 0; i < input.size(); i++) {
            inputNodes[i].weight = (static_cast<float>(input[i].getFloat()) / reduction);
        }

        cpuProcessingUnit.learnNetworkCluster(cluster);
    }

    return true;
}
