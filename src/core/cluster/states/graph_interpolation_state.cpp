/**
 * @file        graph_interpolation_state.cpp
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

#include "graph_interpolation_state.h"

#include <core/segments/dynamic_segment/dynamic_segment.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>

#include <core/cluster/cluster.h>

/**
 * @brief constructor
 *
 * @param cluster pointer to the cluster, where the event and the statemachine belongs to
 */
GraphInterpolation_State::GraphInterpolation_State(Cluster* cluster)
{
    m_cluster = cluster;
}

/**
 * @brief destructor
 */
GraphInterpolation_State::~GraphInterpolation_State() {}

/**
 * @brief prcess event
 *
 * @return alway true
 */
bool
GraphInterpolation_State::processEvent()
{
    Task* actualTask = m_cluster->getActualTask();

    if(actualTask->isInit)
    {
        // set input
        InputNode* inputNodes = m_cluster->inputSegments[0]->inputs;
        for(uint64_t i = 2; i < 10000; i++) {
            inputNodes[i - 2].weight = inputNodes[i].weight;
        }

        OutputNode* outputNodes = m_cluster->outputSegments[0]->outputs;
        inputNodes[10000 - 2].weight = outputNodes[0].outputWeight;
        inputNodes[10000 - 1].weight = outputNodes[1].outputWeight;
    }
    else
    {
        float lastVal = actualTask->inputData[actualTask->actualCycle];
        float actualVal = 0.0f;
        uint64_t pos = actualTask->actualCycle;

        // set input
        InputNode* inputNodes = m_cluster->inputSegments[0]->inputs;
        for(uint64_t i = actualTask->actualCycle + 1;
            i < actualTask->actualCycle + 1 + 5000;
            i++)
        {
            actualVal = actualTask->inputData[i];

            if(actualVal < lastVal)
            {
                inputNodes[pos * 2].weight = actualVal / lastVal;
                inputNodes[pos * 2 + 1].weight = 0.0f;
            }
            else
            {
                inputNodes[pos * 2].weight = 0.0f;
                inputNodes[pos * 2 + 1].weight = lastVal / actualVal;
            }

            lastVal = actualVal;
            pos++;
        }
    }

    actualTask->isInit = true;

    return true;
}
