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

struct NetworkMetaData
{
    float lerningValue = 0.0f;
    uint32_t cycleTime = 1000000;

    uint8_t padding[247];
};

struct InitSettings
{
    uint16_t nodesPerBrick = 0;
    uint16_t maxBrickDistance = 0;
    float nodeLowerBorder = 0.0f;
    float nodeUpperBorder = 0.0f;
    uint32_t layer = 0;
    uint64_t maxSynapseSections = 0;

    uint8_t padding[232];
};

struct NetworkCluster
{
    NetworkMetaData networkMetaData;
    InitSettings initMetaData;

    Segment* synapseSegment = nullptr;

    uint32_t* randomValues = nullptr;
};

#endif // NETWORK_CLUSTER_H
