/**
 * @file        task_queue.cpp
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

#include "task_queue.h"

TaskQueue::TaskQueue() {}

/**
 * @brief TaskQueue::addLearnTask
 * @param data
 * @param numberOfInputsPerCycle
 * @param numberOfOuputsPerCycle
 * @param numberOfCycle
 * @return
 */
const std::string
TaskQueue::addLearnTask(float* data,
                        const uint64_t numberOfInputsPerCycle,
                        const uint64_t numberOfOuputsPerCycle,
                        const uint64_t numberOfCycle)
{
    Task newTask;
    newTask.uuid = generateUuid();
    newTask.data = data;
    newTask.numberOfInputsPerCycle = numberOfInputsPerCycle;
    newTask.numberOfOuputsPerCycle = numberOfOuputsPerCycle;
    newTask.numberOfCycle = numberOfCycle;
    newTask.type = LEARN_TASK;
    newTask.state = QUEUED_TASK_STATE;

    const std::string uuid = newTask.uuid.toString();

    m_mutex.lock();

    m_taskMap.insert(std::make_pair(uuid, newTask));
    m_taskQueue.push_back(uuid);

    m_mutex.unlock();

    return uuid;
}

/**
 * @brief TaskQueue::getState
 * @param taskUuid
 * @return
 */
TaskState
TaskQueue::getState(const std::string &taskUuid)
{
    TaskState state = UNDEFINED_TASK_STATE;

    m_mutex.lock();

    std::map<std::string, Task>::const_iterator it;
    it = m_taskMap.find(taskUuid);

    if(it != m_taskMap.end()) {
        state = it->second.state;
    }

    m_mutex.unlock();

    return state;
}

/**
 * @brief TaskQueue::isFinish
 * @param taskUuid
 * @return
 */
bool
TaskQueue::isFinish(const std::string &taskUuid)
{
    const TaskState state = getState(taskUuid);
    return state == FINISHED_TASK_STATE;
}

/**
 * @brief TaskQueue::removeTask
 * @param taskUuid
 */
void
TaskQueue::removeTask(const std::string &taskUuid)
{
    TaskState state = UNDEFINED_TASK_STATE;

    m_mutex.lock();

    // check and update map
    std::map<std::string, Task>::iterator itMap;
    itMap = m_taskMap.find(taskUuid);
    if(itMap != m_taskMap.end())
    {
        state = itMap->second.state;

        // if only queue but not activly processed at the moment, it can easily deleted
        if(state == QUEUED_TASK_STATE)
        {
            delete itMap->second.data;
            m_taskMap.erase(itMap);
        }

        // if task is active at the moment, then only mark it as aborted
        if(state == ACTIVE_TASK_STATE) {
            itMap->second.state = ABORTED_TASK_STATE;
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

    m_mutex.unlock();
}

/**
 * @brief TaskQueue::getNextTask
 * @return
 */
Task
TaskQueue::getNextTask()
{
    Task result;

    m_mutex.lock();

    if(m_taskQueue.size() == 0) {
        return result;
    }

    const std::string nextUuid = m_taskQueue.front();
    m_taskQueue.pop_front();

    std::map<std::string, Task>::const_iterator it;
    it = m_taskMap.find(nextUuid);

    if(it != m_taskMap.end()) {
        result = it->second;
    }

    m_mutex.unlock();

    return result;
}

/**
 * @brief TaskQueue::finishTask
 * @param taskUuid
 */
void
TaskQueue::finishTask(const std::string &taskUuid)
{
    m_mutex.lock();

    std::map<std::string, Task>::iterator it;
    it = m_taskMap.find(taskUuid);

    if(it != m_taskMap.end())
    {
        delete it->second.data;
        it->second.state = FINISHED_TASK_STATE;
    }

    m_mutex.unlock();
}
