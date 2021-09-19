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

enum TaskType
{
    UNDEFINED_TASK = 0,
    LEARN_TASK = 1,
};

enum TaskState
{
    UNDEFINED_TASK_STATE = 0,
    QUEUED_TASK_STATE = 1,
    ACTIVE_TASK_STATE = 2,
    ABORTED_TASK_STATE = 3,
    FINISHED_TASK_STATE = 4,
};

struct Task
{
    kuuid uuid;
    float* data = nullptr;
    uint64_t numberOfInputsPerCycle = 0;
    uint64_t numberOfOuputsPerCycle = 0;
    uint64_t numberOfCycle = 0;
    uint64_t actualCycle = 0;
    TaskType type = UNDEFINED_TASK;
    TaskState state = UNDEFINED_TASK_STATE;
};

class TaskQueue
{
public:
    TaskQueue();

    const std::string addLearnTask(float* data,
                                   const uint64_t numberOfInputsPerCycle,
                                   const uint64_t numberOfOuputsPerCycle,
                                   const uint64_t numberOfCycle);
    TaskState getState(const std::string &taskUuid);
    bool isFinish(const std::string &taskUuid);
    void removeTask(const std::string &taskUuid);
    Task getNextTask();
    void finishTask(const std::string &taskUuid);

private:
    std::mutex m_mutex;
    std::deque<std::string> m_taskQueue;
    std::map<std::string, Task> m_taskMap;
};

#endif // KYOUKOMIND_TASKQUEUE_H
