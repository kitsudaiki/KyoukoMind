/**
 * @file        task_queue.h
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

#ifndef KYOUKOMIND_TASKQUEUE_H
#define KYOUKOMIND_TASKQUEUE_H

#include <common.h>
#include <core/objects/task.h>

class TaskQueue
{
public:
    TaskQueue();

    const std::string addLearnTask(float* inputData,
                                   float* labels,
                                   const uint64_t numberOfInputsPerCycle,
                                   const uint64_t numberOfOuputsPerCycle,
                                   const uint64_t numberOfCycle);
    const std::string addRequestTask(float* inputData,
                                     const uint64_t numberOfInputsPerCycle,
                                     const uint64_t numberOfCycle);

    TaskState getState(const std::string &taskUuid);
    const TaskProgress getProgress(const std::string &taskUuid);
    uint32_t* getResultData(const std::string &taskUuid);
    uint32_t getResultSize(const std::string &taskUuid);

    bool isFinish(const std::string &taskUuid);
    bool removeTask(const std::string &taskUuid);
    bool getNextTask();
    void finishTask();

    Task* actualTask = nullptr;

private:
    std::deque<std::string> m_taskQueue;
    std::map<std::string, Task> m_taskMap;
};

#endif // KYOUKOMIND_TASKQUEUE_H
