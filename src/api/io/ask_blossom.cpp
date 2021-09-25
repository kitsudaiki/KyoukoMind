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

#include <libKitsunemimiJson/json_item.h>
#include <core/processing/cpu/cpu_processing_unit.h>
#include <core/structure/network_cluster.h>
#include <core/structure/segments/input_segment.h>
#include <core/structure/segments/output_segment.h>
#include <core/orchestration/cluster_handler.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;
using namespace Kitsunemimi::Json;

AskBlossom::AskBlossom()
    : Blossom()
{
    registerField("cluster_uuid", INPUT_TYPE, true);
    registerField("request", INPUT_TYPE, true);
    registerField("response", OUTPUT_TYPE, true);
}

bool
AskBlossom::runTask(BlossomLeaf &blossomLeaf,
                    std::string &errorMessage)
{
    const std::string requestString = blossomLeaf.input.getStringByKey("request");
    JsonItem request;
    if(request.parse(requestString, errorMessage) == false) {
        return false;
    }

    const std::string uuid = blossomLeaf.input.getStringByKey("cluster_uuid");
    ClusterInterface* cluster = KyoukoRoot::m_root->m_clusterHandler->getCluster(uuid);
    // TODO: handle if not found
    CpuProcessingUnit cpuProcessingUnit;

    const uint32_t numberOfInputs = request["number_of_inputs"].getInt();
    const float reduction = request["reduction"].getFloat();

    for(uint32_t pic = 0; pic < numberOfInputs; pic++)
    {
        JsonItem input = request["input"][pic];

       /* for(uint32_t i = 0; i < input.size(); i++) {
            inputNodes[i].weight = (static_cast<float>(input[i].getFloat()) / reduction);
        }*/

        //cpuProcessingUnit.processSegment(cluster);

        DataArray* response = new DataArray();
        /*OutputSegment* synapseSegment = cluster->outputSegments[0];
        for(uint64_t i = 0; i < synapseSegment->segmentHeader->outputs.count; i++)
        {
            OutputNode* out = &synapseSegment->outputs[i];
            response->append(new DataValue(out->outputWeight));
        }*/

        blossomLeaf.output.insert("result", response);
    }

    return true;
}
