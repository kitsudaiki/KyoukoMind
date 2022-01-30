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

class AbstractSegment;
class InputSegment;
class OutputSegment;
class AbstractSegment;

class Cluster
{
public:
    Cluster();
    ~Cluster();

    // cluster-data
    Kitsunemimi::DataBuffer clusterData;
    ClusterMetaData* networkMetaData = nullptr;
    ClusterSettings* networkSettings = nullptr;
    std::vector<InputSegment*> inputSegments;
    std::vector<OutputSegment*> outputSegments;
    std::vector<AbstractSegment*> allSegments;

    // init
    const std::string getName();
    bool setName(const std::string newName);
    bool init(const JsonItem &parsedContent,
              const std::string &uuid);

    // task-handling
    void updateClusterState();
    const std::string addLearnTask(float* inputData,
                                   const uint64_t numberOfInputsPerCycle,
                                   const uint64_t numberOfOuputsPerCycle,
                                   const uint64_t numberOfCycle);
    const std::string addRequestTask(float* inputData,
                                     const uint64_t numberOfInputsPerCycle,
                                     const uint64_t numberOfCycle);
    uint32_t request(float* inputData, const uint64_t numberOfInputes);

    // tasks
    uint64_t getActualTaskCycle();
    const TaskProgress getProgress(const std::string &taskUuid);
    const uint32_t* getResultData(const std::string &taskUuid);
    uint32_t getResultSize(const std::string &taskUuid);
    bool removeTask(const std::string &taskUuid);
    bool isFinish(const std::string &taskUuid);
    void setResultForActualCycle(const uint32_t result);

    enum ClusterMode
    {
        NORMAL_MODE = 0,
        LEARN_FORWARD_MODE = 1,
        LEARN_BACKWARD_MODE = 2
    };
    ClusterMode getMode() const;
    uint32_t m_segmentCounter = 0;

private:
    // task
    Task* actualTask = nullptr;
    std::deque<std::string> m_taskQueue;
    std::map<std::string, Task> m_taskMap;
    std::mutex m_task_mutex;

    TaskState getTaskState(const std::string &taskUuid);
    bool getNextTask();
    void finishTask();

    ClusterMode m_mode = NORMAL_MODE;

    void startForwardLearnCycle();
    void startBackwardLearnCycle();
};

#endif // KYOUKOMIND_CLUSTER_INTERFACE_H
