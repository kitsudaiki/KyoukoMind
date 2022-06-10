/**
 * @file        task.h
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

#ifndef KYOUKOMIND_TASK_H
#define KYOUKOMIND_TASK_H

#include <common.h>

enum TaskType
{
    UNDEFINED_TASK = 0,
    IMAGE_LEARN_TASK = 1,
    IMAGE_REQUEST_TASK = 2,
    GRAPH_LEARN_TASK = 3,
    GRAPH_REQUEST_TASK = 4,
    CLUSTER_SNAPSHOT_SAVE_TASK = 5,
    CLUSTER_SNAPSHOT_RESTORE_TASK = 6,
};

enum TaskState
{
    UNDEFINED_TASK_STATE = 0,
    QUEUED_TASK_STATE = 1,
    ACTIVE_TASK_STATE = 2,
    ABORTED_TASK_STATE = 3,
    FINISHED_TASK_STATE = 4,
};

struct TaskProgress
{
    TaskState state = UNDEFINED_TASK_STATE;
    float percentageFinished = 0.0f;
    std::chrono::high_resolution_clock::time_point queuedTimeStamp;
    std::chrono::high_resolution_clock::time_point startActiveTimeStamp;
    std::chrono::high_resolution_clock::time_point endActiveTimeStamp;
    uint64_t estimatedRemaningTime = 0;
};

struct Task
{
    Kitsunemimi::Hanami::kuuid uuid;
    bool isInit = false;
    float* inputData = nullptr;
    DataArray* resultData = nullptr;
    DataMap metaData;
    uint64_t actualCycle = 0;
    TaskType type = UNDEFINED_TASK;
    TaskProgress progress;

    uint64_t getIntVal(const std::string &name)
    {
        return static_cast<uint64_t>(metaData.get(name)->toValue()->getLong());
    }
};

#endif // KYOUKOMIND_TASK_H
