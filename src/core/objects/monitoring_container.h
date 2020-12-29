/**
 * @file        kyouko_root.cpp
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

#ifndef MONITORINGCONTAINER_H
#define MONITORINGCONTAINER_H

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>

enum MessageTypes
{
    UNDEFINED_TYPE = 0,
    MONITORING_BRICK_DATA,
    MONITORING_META_DATA,
};

struct MonitoringBrick
{
    uint32_t brickId = 0;
    uint32_t xPos = 0;
    uint32_t yPos = 0;
    float activityValue = 0.0f;
};

struct MonitoringBrickMessage
{
    uint8_t messageType = MONITORING_BRICK_DATA;
    uint32_t numberOfInfos = 0;
    MonitoringBrick brickInfos[1000];
};

struct MonitoringProcessingCount
{
    uint64_t numberOfAxonMessages = 0;
    uint64_t numberOfUpdateMessages = 0;
    uint64_t numberOfSynapseMessages = 0;
    uint64_t numberOfActiveNodes = 0;
};

struct MonitoringProcessingTimes
{
    float edgePhase = 0.0f;
    float synapsePhase = 0.0f;
    float totalCycle = 0.0f;

    float cpuUpdate = 0.0f;
    float cpuProcessing = 0.0f;

    float copyToGpu = 0.0f;
    float gpuSynapse = 0.0f;
    float gpuNode = 0.0f;
    float gpuUpdate = 0.0f;
    float copyFromGpu = 0.0f;
    float cleanup = 0.0f;

    uint32_t axonTransfers = 0;
    uint32_t updateTransfers = 0;
    uint64_t nodes = 0;
    uint64_t synapseSections = 0;
    uint64_t edgeSections = 0;


    void reset()
    {
        edgePhase = 0.0f;
        synapsePhase = 0.0f;
        totalCycle = 0.0f;

        cpuUpdate = 0.0f;
        cpuProcessing = 0.0f;

        copyToGpu = 0.0;
        gpuSynapse = 0.0f;
        gpuNode = 0.0f;
        gpuUpdate = 0.0f;
        copyFromGpu = 0.0;
        cleanup = 0.0;

        axonTransfers = 0;
        updateTransfers = 0;
        nodes = 0;
        synapseSections = 0;
        edgeSections = 0;
    }

    const std::string toString()
    {
        const std::string meta =
                "{\"processing\": { "
                "\"edgePhase\":" + std::to_string(edgePhase) + ","
                "\"synapsePhase\":" + std::to_string(synapsePhase) + ","
                "\"totalCycle\":" + std::to_string(totalCycle) + ","
                "\"cpuUpdate\":" + std::to_string(cpuUpdate) + ","
                "\"cpuProcessing\":" + std::to_string(cpuProcessing) + ","
                "\"copyToGpu\":" + std::to_string(copyToGpu) + ","
                "\"gpuSynapse\":" + std::to_string(gpuSynapse) + ","
                "\"gpuNode\":" + std::to_string(gpuNode) + ","
                "\"gpuUpdate\":" + std::to_string(gpuUpdate) + ","
                "\"copyFromGpu\":" + std::to_string(copyFromGpu) + ","
                "\"cleanup\":" + std::to_string(cleanup) + ","
                "\"axonTransfers\":" + std::to_string(axonTransfers) + ","
                "\"updateTransfers\":" + std::to_string(updateTransfers) + ","
                "\"nodes\":" + std::to_string(nodes) + ","
                "\"synapseSections\":" + std::to_string(synapseSections) + ","
                "\"edgeSections\":" + std::to_string(edgeSections) +
                "}}";
        return meta;
    }
};

#endif // MONITORINGCONTAINER_H
