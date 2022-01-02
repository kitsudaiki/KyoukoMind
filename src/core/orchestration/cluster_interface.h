/**
 * @file        cluster_interface.h
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

#ifndef KYOUKOMIND_CLUSTER_INTERFACE_H
#define KYOUKOMIND_CLUSTER_INTERFACE_H

#include <common.h>
#include <core/objects/task.h>
#include <core/objects/cluster_meta.h>

class TaskQueue;
class NetworkCluster;
class AbstractSegment;

enum ClusterMode
{
    NORMAL_MODE = 0,
    LEARN_FORWARD_MODE = 1,
    LEARN_BACKWARD_MODE = 2
};

class ClusterInterface
{
public:
    ClusterInterface();

    bool initNewCluster(const JsonItem &parsedContent,
                        const std::string &uuid);

    // task-handling
    void updateClusterState();
    const std::string addLearnTask(float* inputData,
                                   float* labels,
                                   const uint64_t numberOfInputsPerCycle,
                                   const uint64_t numberOfOuputsPerCycle,
                                   const uint64_t numberOfCycle);
    const std::string addRequestTask(float* inputData,
                                     const uint64_t numberOfInputsPerCycle,
                                     const uint64_t numberOfCycle);
    uint32_t request(float* inputData, const uint64_t numberOfInputes);

    uint64_t getActualTaskCycle();
    const TaskProgress getProgress(const std::string &taskUuid);
    const uint32_t* getResultData(const std::string &taskUuid);
    uint32_t getResultSize(const std::string &taskUuid);
    bool removeResultData(const std::string &taskUuid);
    bool isFinish(const std::string &taskUuid);
    void setResultForActualCycle(const uint32_t result);

    // cluster-handling
    uint64_t getNumberOfSegments() const;
    AbstractSegment* getSegment(const uint64_t id) const;

    ClusterMode getMode() const;
    uint32_t m_segmentCounter = 0;

private:
    TaskQueue* m_taskQueue = nullptr;
    NetworkCluster* m_cluster = nullptr;

    std::mutex m_task_mutex;
    ClusterMode m_mode = NORMAL_MODE;

    void startForwardLearnCycle();
    void startBackwardLearnCycle();
};

#endif // KYOUKOMIND_CLUSTER_INTERFACE_H