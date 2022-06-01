/**
 * @file        task_handle_state.cpp
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

#include "task_handle_state.h"

#include <core/cluster/cluster.h>
#include <libSagiriArchive/sagiri_send.h>

/**
 * @brief constructor
 *
 * @param cluster pointer to the cluster, where the event and the statemachine belongs to
 */
TaskHandle_State::TaskHandle_State(Cluster* cluster)
{
    m_cluster = cluster;
}

/**
 * @brief destructor
 */
TaskHandle_State::~TaskHandle_State() {}

/**
 * @brief prcess event
 *
 * @return false, if statechange failed, else true
 */
bool
TaskHandle_State::processEvent()
{
    m_task_mutex.lock();
    finishTask();
    const bool hasNextState = getNextTask();
    m_task_mutex.unlock();

    if(hasNextState)
    {
        switch(actualTask->type)
        {
            case IMAGE_LEARN_TASK:
            {
                if(m_cluster->goToNextState(Cluster::LEARN)) {
                    m_cluster->goToNextState(Cluster::IMAGE);
                } else {
                    // TODO: error-message
                    return false;
                }
                break;
            }
            case IMAGE_REQUEST_TASK:
            {
                if(m_cluster->goToNextState(Cluster::REQUEST)) {
                    m_cluster->goToNextState(Cluster::IMAGE);
                } else {
                    // TODO: error-message
                    return false;
                }
                break;
            }
            case GRAPH_LEARN_TASK:
            {
                if(m_cluster->goToNextState(Cluster::LEARN)) {
                    m_cluster->goToNextState(Cluster::GRAPH);
                } else {
                    // TODO: error-message
                    return false;
                }
                break;
            }
            case GRAPH_REQUEST_TASK:
            {
                if(m_cluster->goToNextState(Cluster::REQUEST)) {
                    m_cluster->goToNextState(Cluster::GRAPH);
                } else {
                    // TODO: error-message
                    return false;
                }
                break;
            }
            case CLUSTER_SNAPSHOT_SAVE_TASK:
            {
                if(m_cluster->goToNextState(Cluster::SNAPSHOT)) {
                    if(m_cluster->goToNextState(Cluster::CLUSTER)) {
                        m_cluster->goToNextState(Cluster::SAVE);
                    } else {
                        // TODO: error-message
                        return false;
                    }
                } else {
                    // TODO: error-message
                    return false;
                }
                break;
            }
            case CLUSTER_SNAPSHOT_RESTORE_TASK:
            {
                if(m_cluster->goToNextState(Cluster::SNAPSHOT)) {
                    if(m_cluster->goToNextState(Cluster::CLUSTER)) {
                        m_cluster->goToNextState(Cluster::RESTORE);
                    } else {
                        // TODO: error-message
                        return false;
                    }
                } else {
                    // TODO: error-message
                    return false;
                }
                break;
            }
            default: {
                // TODO: error-message
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief add new task
 *
 * @param uuid uuid of the new task for identification
 * @param task task itself
 *
 * @return alsways true
 */
bool
TaskHandle_State::addTask(const std::string &uuid,
                          const Task &task)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    // TODO: check if uuid already exist (update comment)
    m_taskMap.insert(std::make_pair(uuid, task));
    m_taskQueue.push_back(uuid);

    return true;
}

/**
 * @brief get pointer to the actual active task
 *
 * @return pointer to the actual task of nullptr, if no task is active at the moment
 */
Task*
TaskHandle_State::getActualTask()
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    return actualTask;
}

/**
 * @brief run next task from the queue
 *
 * @return false, if task-queue if empty, else true
 */
bool
TaskHandle_State::getNextTask()
{
    // check number of tasks in queue
    if(m_taskQueue.size() == 0) {
        return false;
    }

    // remove task from queue
    const std::string nextUuid = m_taskQueue.front();
    m_taskQueue.pop_front();

    // init the new task
    std::map<std::string, Task>::iterator it;
    it = m_taskMap.find(nextUuid);
    it->second.progress.state = ACTIVE_TASK_STATE;
    it->second.progress.startActiveTimeStamp = std::chrono::system_clock::now();
    actualTask = &it->second;

    return true;
}

/**
 * @brief finish actual task
 */
void
TaskHandle_State::finishTask()
{
    // precheck
    if(actualTask == nullptr) {
        return;
    }

    // send results to sagiri, if some are attached to the task
    if(actualTask->resultData != nullptr)
    {
        Kitsunemimi::ErrorContainer error;
        if(Sagiri::sendResults(actualTask->uuid.toString(),
                               *actualTask->resultData,
                               error) == false)
        {
            LOG_ERROR(error);
        }
        delete actualTask->resultData;
        actualTask->resultData = nullptr;
    }

    // remove task from map and free its data
    std::map<std::string, Task>::iterator it;
    it = m_taskMap.find(actualTask->uuid.toString());
    if(it != m_taskMap.end())
    {
        delete it->second.inputData;
        it->second.progress.state = FINISHED_TASK_STATE;
        it->second.progress.endActiveTimeStamp = std::chrono::system_clock::now();
    }

    actualTask = nullptr;
}

/**
 * @brief get task-progress
 *
 * @param taskUuid UUID of the task
 *
 * @return task-progress
 */
const TaskProgress
TaskHandle_State::getProgress(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    std::map<std::string, Task>::const_iterator it;
    it = m_taskMap.find(taskUuid);
    if(it != m_taskMap.end()) {
        return it->second.progress;
    }

    TaskProgress progress;
    return progress;
}

/**
 * @brief get state of a task
 *
 * @param taskUuid UUID of the task
 *
 * @return state of the requested task
 */
TaskState
TaskHandle_State::getTaskState(const std::string &taskUuid)
{
    TaskState state = UNDEFINED_TASK_STATE;

    std::map<std::string, Task>::const_iterator it;
    it = m_taskMap.find(taskUuid);
    if(it != m_taskMap.end()) {
        state = it->second.progress.state;
    }

    return state;
}

/**
 * @brief remove task from queue of abort the task, if actual in progress
 *
 * @param taskUuid UUID of the task
 *
 * @return always true
 */
bool
TaskHandle_State::removeTask(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    TaskState state = UNDEFINED_TASK_STATE;

    // check and update map
    std::map<std::string, Task>::iterator itMap;
    itMap = m_taskMap.find(taskUuid);
    if(itMap != m_taskMap.end())
    {
        state = itMap->second.progress.state;

        // if only queue but not activly processed at the moment, it can easily deleted
        if(state == QUEUED_TASK_STATE)
        {
            delete itMap->second.inputData;
            m_taskMap.erase(itMap);
        }

        // if task is active at the moment, then only mark it as aborted
        if(state == ACTIVE_TASK_STATE) {
            itMap->second.progress.state = ABORTED_TASK_STATE;
        }
    }

    // check and update queue
    if(state == QUEUED_TASK_STATE)
    {
        std::deque<std::string>::const_iterator itQueue;
        itQueue = std::find(m_taskQueue.begin(), m_taskQueue.end(), taskUuid);
        if(itQueue != m_taskQueue.end()) {
            m_taskQueue.erase(itQueue);
        }
    }

    return true;
}

/**
 * @brief check if a task is finished
 *
 * @param taskUuid UUID of the task
 *
 * @return true, if task is finished, else false
 */
bool
TaskHandle_State::isFinish(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    return getTaskState(taskUuid) == FINISHED_TASK_STATE;
}

/**
 * @brief add new value to the list of results in the current task
 *
 * @param result new resulting values of the actual cycle
 */
void
TaskHandle_State::setResultForActualCycle(const uint32_t result)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    if(actualTask->resultData == nullptr) {
        return;
    }

    actualTask->resultData->append(new DataValue(static_cast<long>(result)));
}
