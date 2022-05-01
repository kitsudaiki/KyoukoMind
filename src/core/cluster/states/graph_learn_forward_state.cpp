/**
 * @file        graph_learn_forward_state.cpp
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

#include "graph_learn_forward_state.h"

#include <core/segments/dynamic_segment/dynamic_segment.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>

#include <core/cluster/cluster.h>

/**
 * @brief constructor
 *
 * @param cluster pointer to the cluster, where the event and the statemachine belongs to
 */
GraphLearnForward_State::GraphLearnForward_State(Cluster* cluster)
{
    m_cluster = cluster;
}

/**
 * @brief destructor
 */
GraphLearnForward_State::~GraphLearnForward_State() {}

/**
 * @brief prcess event
 *
 * @return alway true
 */
bool
GraphLearnForward_State::processEvent()
{
    std::cout<<"fillGraphLernBuffer"<<std::endl;
    Task* actualTask = m_cluster->getActualTask();

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
            inputNodes[pos * 2].weight = actualVal;
            inputNodes[pos * 2 + 1].weight = 0.0f;
        }
        else
        {
            inputNodes[pos * 2].weight = 0.0f;
            inputNodes[pos * 2 + 1].weight = actualVal;
        }

        if(pos == actualTask->actualCycle) {
            std::cout<<"pos: "<<actualTask->actualCycle<<"    val: "<<inputNodes[pos * 2].weight<<" : "<<inputNodes[pos * 2 + 1].weight<<std::endl;
        }

        lastVal = actualVal;
        pos++;
    }

    // set exprected output
    OutputNode* outputNodes = m_cluster->outputSegments[0]->outputs;
    actualVal = actualTask->inputData[actualTask->actualCycle + 1 + 5000];

    if(actualVal < lastVal)
    {
        outputNodes[0].shouldValue = actualVal;
        outputNodes[1].shouldValue = 0.0f;
    }
    else
    {
        outputNodes[0].shouldValue = 0.0f;
        outputNodes[1].shouldValue = actualVal;
    }

    return true;
}
