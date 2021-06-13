/**
 * @file        static_processing.cpp
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

#include "static_processing.h"

#include <kyouko_root.h>

#include <core/objects/segment.h>
#include <core/objects/network_cluster.h>

#include <core/processing/cpu/processing.h>
#include <core/processing/cpu/io.h>
#include <core/processing/cpu/backpropagation.h>

#include <libKitsunemimiCommon/threading/barrier.h>

StaticProcessing::StaticProcessing()
{
}

StaticProcessing::~StaticProcessing()
{

}

/**
 * @brief StaticProcessing::learn
 * @return
 */
bool
StaticProcessing::learn()
{
    return learnStep();
}

/**
 * @brief StaticProcessing::execute
 * @return
 */
bool
StaticProcessing::execute()
{
    executeStep();
    return true;
}

/**
 * @brief Lerner::learnStep
 * @return
 */
bool
StaticProcessing::learnStep()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    cluster->networkMetaData.doLearn = 1;

    updateLearning();

    cluster->networkMetaData.doLearn = 0;
    cluster->networkMetaData.lerningValue = 0.0f;

    return true;
}
