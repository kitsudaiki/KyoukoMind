/**
 * @file        cluster_interface.cpp
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

#include "cluster.h"

#include <core/data_structure/segments/dynamic_segment.h>
#include <core/data_structure/segments/input_segment.h>
#include <core/data_structure/segments/output_segment.h>

#include <core/processing/segment_queue.h>
#include <core/processing/cpu/output_segment/processing.h>

#include <libKitsunemimiCommon/logger.h>
#include <kyouko_root.h>

#include <core/data_structure/init/cluster_init.h>

/**
 * @brief constructor
 */
Cluster::Cluster() {}

/**
 * @brief destructor
 */
Cluster::~Cluster()
{
    for(AbstractSegment* segment : allSegments) {
        delete segment;
    }
}

/**
 * @brief init the cluster
 *
 * @param parsedContent parsed json-content with structural information of the new cluster
 * @param uuid UUID of the new cluster
 *
 * @return true, if successful, else false
 */
bool
Cluster::init(const JsonItem &parsedContent,
              const std::string &uuid)
{
    return initNewCluster(this, parsedContent, uuid);
}

/**
 * @brief get the name of the clsuter
 *
 * @return name of the cluster
 */
const std::string
Cluster::getName()
{
    // precheck
    if(networkMetaData == nullptr) {
        return std::string("");
    }

    return std::string(networkMetaData->name);
}

/**
 * @brief set new name for the cluster
 *
 * @param newName new name
 *
 * @return true, if successful, else false
 */
bool
Cluster::setName(const std::string newName)
{
    // precheck
    if(networkMetaData == nullptr
            || newName.size() > 1023
            || newName.size() == 0)
    {
        return false;
    }

    // copy string into char-buffer and set explicit the escape symbol to be absolut sure
    // that it is set to absolut avoid buffer-overflows
    strncpy(networkMetaData->name, newName.c_str(), newName.size());
    networkMetaData->name[newName.size()] = '\0';

    return true;
}

/**
 * @brief start a new forward learn-cycle
 */
void
Cluster::startForwardLearnCycle()
{
    const uint64_t offsetInput = actualTask->numberOfInputsPerCycle * actualTask->actualCycle;
    const uint64_t offsetLabels = actualTask->numberOfOuputsPerCycle * actualTask->actualCycle;

    // set cluster mode
    if(actualTask->type == LEARN_TASK) {
        m_mode = LEARN_FORWARD_MODE;
    }

    // set input
    InputNode* inputNodes = inputSegments[0]->inputs;
    for(uint64_t i = 0; i < actualTask->numberOfInputsPerCycle; i++) {
        inputNodes[i].weight = actualTask->inputData[offsetInput + i];
    }

    // set exprected output
    OutputNode* outputNodes = outputSegments[0]->outputs;
    for(uint64_t i = 0; i < actualTask->numberOfOuputsPerCycle; i++) {
        outputNodes[i].shouldValue = actualTask->labels[offsetLabels + i];
    }

    // set ready-states of all neighbors of all segments
    for(AbstractSegment* segment: allSegments)
    {
        for(uint8_t side = 0; side < 12; side++)
        {
            SegmentNeighbor* neighbor = &segment->segmentNeighbors->neighbors[side];
            neighbor->inputReady = true;
            if(neighbor->direction == INPUT_DIRECTION) {
                neighbor->inputReady = false;
            }
        }
    }

    KyoukoRoot::m_segmentQueue->addSegmentListToQueue(allSegments);
}

/**
 * @brief start a new backward learn-cycle
 */
void
Cluster::startBackwardLearnCycle()
{
    // set cluster mode
    m_mode = LEARN_BACKWARD_MODE;

    // set ready-states of all neighbors of all segments
    for(AbstractSegment* segment: allSegments)
    {
        for(uint8_t side = 0; side < 12; side++)
        {
            SegmentNeighbor* neighbor = &segment->segmentNeighbors->neighbors[side];
            neighbor->inputReady = true;
            if(neighbor->direction == OUTPUT_DIRECTION) {
                neighbor->inputReady = false;
            }
        }
    }

    KyoukoRoot::m_segmentQueue->addSegmentListToQueue(allSegments);
}

/**
 * @brief NetworkCluster::updateClusterState
 */
void
Cluster::updateClusterState()
{
    m_segmentCounter++;
    if(m_segmentCounter < allSegments.size()) {
        return;
    }

    m_segmentCounter = 0;

    if(m_mode == LEARN_FORWARD_MODE)
    {
        // calc error of clearning-step and make back-propagation if necessary
        const float error = calcTotalError(outputSegments[0]);
        if(error > 0.1f) // TODO: make configurable
        {
            startBackwardLearnCycle();
            return;
        }
    }

    std::lock_guard<std::mutex> guard(m_task_mutex);

    m_mode = NORMAL_MODE;

    if(actualTask == nullptr)
    {
        // get next task
        if(getNextTask()) {
            startForwardLearnCycle();
        }
    }
    else
    {
        actualTask->actualCycle++;
        const float actualF = static_cast<float>(actualTask->actualCycle);
        const float shouldF = static_cast<float>(actualTask->numberOfCycle);
        actualTask->progress.percentageFinished = actualF / shouldF;

        if(actualTask->actualCycle == actualTask->numberOfCycle)
        {
            finishTask();
            if(getNextTask()) {
                startForwardLearnCycle();
            }
        }
        else
        {
            startForwardLearnCycle();
        }
    }
}

/**
 * @brief get actual cluster-mode
 *
 * @return actual cluster-mode
 */
Cluster::ClusterMode
Cluster::getMode() const
{
    return m_mode;
}

/**
 * @brief create a learn-task and add it to the task-queue
 *
 * @param inputData input-data
 * @param labels label-data
 * @param numberOfInputsPerCycle number of inputs, which belongs to one cycle
 * @param numberOfOuputsPerCycle number of outputs, which belongs to one cycle
 * @param numberOfCycle number of cycles
 *
 * @return task-uuid
 */
const std::string
Cluster::addLearnTask(float* inputData,
                      float* labels,
                      const uint64_t numberOfInputsPerCycle,
                      const uint64_t numberOfOuputsPerCycle,
                      const uint64_t numberOfCycle)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    // create new learn-task
    Task newTask;
    newTask.uuid = Kitsunemimi::Hanami::generateUuid();
    newTask.inputData = inputData;
    newTask.labels = labels;
    newTask.numberOfInputsPerCycle = numberOfInputsPerCycle;
    newTask.numberOfOuputsPerCycle = numberOfOuputsPerCycle;
    newTask.numberOfCycle = numberOfCycle;
    newTask.type = LEARN_TASK;
    newTask.progress.state = QUEUED_TASK_STATE;
    newTask.progress.queuedTimeStamp = std::chrono::system_clock::now();

    const std::string uuid = newTask.uuid.toString();

    // add task to queue
    m_taskMap.insert(std::make_pair(uuid, newTask));
    m_taskQueue.push_back(uuid);

    return uuid;
}

/**
 * @brief create a request-task and add it to the task-queue
 *
 * @param inputData input-data
 * @param numberOfInputsPerCycle number of inputs, which belongs to one cycle
 * @param numberOfCycle number of cycles
 *
 * @return task-uuid
 */
const std::string
Cluster::addRequestTask(float* inputData,
                        const uint64_t numberOfInputsPerCycle,
                        const uint64_t numberOfCycle)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    // create new request-task
    Task newTask;
    newTask.uuid = Kitsunemimi::Hanami::generateUuid();
    newTask.inputData = inputData;
    newTask.resultData = new uint32_t[numberOfCycle];
    newTask.numberOfInputsPerCycle = numberOfInputsPerCycle;
    newTask.numberOfCycle = numberOfCycle;
    newTask.type = REQUEST_TASK;
    newTask.progress.state = QUEUED_TASK_STATE;
    newTask.progress.queuedTimeStamp = std::chrono::system_clock::now();

    const std::string uuid = newTask.uuid.toString();

    // add task to queue
    m_taskMap.insert(std::make_pair(uuid, newTask));
    m_taskQueue.push_back(uuid);

    return uuid;
}

/**
 * @brief run request-task with only one cycle
 *
 * @param inputData input-data
 * @param numberOfInputsPerCycle number of inputs, which belongs to one cycle
 *
 * @return result of the request
 */
uint32_t
Cluster::request(float* inputData,
                 const uint64_t numberOfInputes)
{
    // create new small request-task
    const std::string taskUuid = addRequestTask(inputData, numberOfInputes, 1);
    m_segmentCounter = allSegments.size();
    updateClusterState();

    // wait until task is finished
    while(isFinish(taskUuid) == false) {
        usleep(10000);
    }

    // get result and clear backend
    const uint32_t result = getResultData(taskUuid)[0];
    removeTask(taskUuid);

    return result;
}

/**
 * @brief run next task from the queue
 *
 * @return false, if task-queue if empty, else true
 */
bool
Cluster::getNextTask()
{
    // HINT: mutex already locked in function 'updateClusterState'

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
Cluster::finishTask()
{
    // HINT: mutex already locked in function 'updateClusterState'

    // precheck
    if(actualTask == nullptr) {
        return;
    }

    // remove task from map and free its data
    std::map<std::string, Task>::iterator it;
    it = m_taskMap.find(actualTask->uuid.toString());
    if(it != m_taskMap.end())
    {
        delete it->second.inputData;
        delete it->second.labels;
        it->second.progress.state = FINISHED_TASK_STATE;
        it->second.progress.endActiveTimeStamp = std::chrono::system_clock::now();
        actualTask = nullptr;
    }
}

/**
 * @brief get cycle of the actual task
 *
 * @return cycle of the actual task
 */
uint64_t
Cluster::getActualTaskCycle()
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    return actualTask->actualCycle;
}

/**
 * @brief get task-progress
 *
 * @param taskUuid UUID of the task
 *
 * @return task-progress
 */
const TaskProgress
Cluster::getProgress(const std::string &taskUuid)
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
 * @brief get result of a task
 *
 * @param taskUuid UUID of the task
 *
 * @return result of the task
 */
const uint32_t*
Cluster::getResultData(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    std::map<std::string, Task>::const_iterator it;
    it = m_taskMap.find(taskUuid);
    if(it != m_taskMap.end()) {
        return it->second.resultData;
    }

    return nullptr;
}

/**
 * @brief get size of task-result
 *
 * @param taskUuid UUID of the task
 *
 * @return size of task-result
 */
uint32_t
Cluster::getResultSize(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    std::map<std::string, Task>::const_iterator it;
    it = m_taskMap.find(taskUuid);
    if(it != m_taskMap.end()) {
        return it->second.numberOfCycle;
    }

    return UNINIT_STATE_32;
}

/**
 * @brief get state of a task
 *
 * @param taskUuid UUID of the task
 *
 * @return state of the requested task
 */
TaskState
Cluster::getTaskState(const std::string &taskUuid)
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
Cluster::removeTask(const std::string &taskUuid)
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
            delete itMap->second.labels;
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
Cluster::isFinish(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    return getTaskState(taskUuid) == FINISHED_TASK_STATE;
}

/**
 * @brief NetworkCluster::setResultForActualCycle
 *
 * @param result
 */
void
Cluster::setResultForActualCycle(const uint32_t result)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);

    actualTask->resultData[actualTask->actualCycle] = result;
}
