/**
 * @file        image_learn_backward_state.cpp
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

#include "image_learn_backward_state.h"

#include <core/cluster/cluster.h>

/**
 * @brief constructor
 *
 * @param cluster pointer to the cluster, where the event and the statemachine belongs to
 */
ImageLearnBackward_State::ImageLearnBackward_State(Cluster* cluster)
{
    m_cluster = cluster;
}

/**
 * @brief destructor
 */
ImageLearnBackward_State::~ImageLearnBackward_State() {}

/**
 * @brief prcess event
 *
 * @return alway true
 */
bool
ImageLearnBackward_State::processEvent()
{
    m_cluster->mode = Cluster::LEARN_BACKWARD_MODE;
    m_cluster->startBackwardCycle();

    return true;
}
