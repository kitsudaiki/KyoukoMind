/**
 * @file        network_cluster.h
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

#ifndef NETWORK_CLUSTER_H
#define NETWORK_CLUSTER_H

#include <common.h>

#include <core/objects/segment.h>
#include <libKitsunemimiAiCommon/metadata.h>

//==================================================================================================

struct NetworkCluster
{
    Kitsunemimi::Ai::NetworkMetaData networkMetaData;
    Kitsunemimi::Ai::InitMeataData initMetaData;

    CoreSegment* synapseSegment = nullptr;
    OutputSegment* outputSegment = nullptr;
};

//==================================================================================================

#endif // NETWORK_CLUSTER_H
