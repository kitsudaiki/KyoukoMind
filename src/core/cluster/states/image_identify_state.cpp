/**
 * @file        image_identify_state.cpp
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

#include "image_identify_state.h"

#include <core/segments/dynamic_segment/dynamic_segment.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>

#include <core/cluster/cluster.h>

/**
 * @brief constructor
 *
 * @param cluster pointer to the cluster, where the event and the statemachine belongs to
 */
ImageIdentify_State::ImageIdentify_State(Cluster* cluster)
{
    m_cluster = cluster;
}

/**
 * @brief destructor
 */
ImageIdentify_State::~ImageIdentify_State() {}

/**
 * @brief prcess event
 *
 * @return alway true
 */
bool
ImageIdentify_State::processEvent()
{
    Task* actualTask = m_cluster->getActualTask();
    const uint64_t numberOfInputsPerCycle = actualTask->getIntVal("number_of_inputs_per_cycle");
    const uint64_t numberOfOuputsPerCycle = actualTask->getIntVal("number_of_outputs_per_cycle");
    const uint64_t entriesPerCycle = numberOfInputsPerCycle + numberOfOuputsPerCycle;
    const uint64_t offsetInput = entriesPerCycle * actualTask->actualCycle;

    // set input
    InputNode* inputNodes = m_cluster->inputSegments[0]->inputs;
    for(uint64_t i = 0; i < numberOfInputsPerCycle; i++) {
        inputNodes[i].weight = actualTask->inputData[offsetInput + i];
    }

    m_cluster->mode = Cluster::NORMAL_MODE;
    m_cluster->startForwardCycle();

    return true;
}
