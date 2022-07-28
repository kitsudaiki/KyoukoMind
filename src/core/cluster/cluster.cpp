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
#include <kyouko_root.h>

#include <core/segments/dynamic_segment/dynamic_segment.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>
#include <core/cluster/cluster_init.h>
#include <core/cluster/statemachine_init.h>

#include <core/cluster/states/task_handle_state.h>

#include <core/processing/segment_queue.h>
#include <core/segments/output_segment/processing.h>

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiCommon/statemachine.h>
#include <libKitsunemimiCommon/threading/thread.h>

/**
 * @brief constructor
 */
Cluster::Cluster()
{
    m_stateMachine = new Kitsunemimi::Statemachine();
    m_taskHandleState = new TaskHandle_State(this);

    initStatemachine(*m_stateMachine, this, m_taskHandleState);
}

/**
 * @brief destructor
 */
Cluster::~Cluster()
{
    delete m_stateMachine;

    // already deleted in the destructor of the statemachine
    // delete m_taskHandleState;

    for(AbstractSegment* segment : allSegments) {
        delete segment;
    }
}

/**
 * @brief get uuid of the cluster
 *
 * @return uuid of the cluster
 */
const
std::string Cluster::getUuid()
{
    return networkMetaData->uuid.toString();
}

/**
 * @brief init the cluster
 *
 * @param parsedContent TODO
 * @param segmentTemplates TODO
 * @param uuid UUID of the new cluster
 *
 * @return true, if successful, else false
 */
bool
Cluster::init(const JsonItem &parsedContent,
              const std::map<std::string, Kitsunemimi::Json::JsonItem> &segmentTemplates,
              const std::string &uuid)
{
    return initNewCluster(this, parsedContent, segmentTemplates, uuid);
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
Cluster::startForwardCycle()
{
    // set ready-states of all neighbors of all segments
    for(AbstractSegment* segment : allSegments)
    {
        for(uint8_t side = 0; side < 12; side++)
        {
            SegmentNeighbor* neighbor = &segment->segmentNeighbors->neighbors[side];
            // TODO: check possible crash here
            neighbor->inputReady = neighbor->direction != INPUT_DIRECTION;
        }
    }

    segmentCounter = 0;
    KyoukoRoot::m_segmentQueue->addSegmentListToQueue(allSegments);
}

/**
 * @brief start a new backward learn-cycle
 */
void
Cluster::startBackwardCycle()
{
    // set ready-states of all neighbors of all segments
    for(AbstractSegment* segment : allSegments)
    {
        for(uint8_t side = 0; side < 12; side++)
        {
            SegmentNeighbor* neighbor = &segment->segmentNeighbors->neighbors[side];
            neighbor->inputReady = neighbor->direction != OUTPUT_DIRECTION;
        }
    }

    segmentCounter = 0;
    KyoukoRoot::m_segmentQueue->addSegmentListToQueue(allSegments);
}

/**
 * @brief switch state of the cluster between task and direct mode
 *
 * @param newState new desired state
 *
 * @return true, if switch in statemachine was successful, else false
 */
bool
Cluster::setClusterState(const std::string &newState)
{
    if(newState == "DIRECT") {
        return goToNextState(SWITCH_TO_DIRECT_MODE);
    }

    if(newState == "TASK") {
        return goToNextState(SWITCH_TO_TASK_MODE);
    }

    return false;
}

/**
 * @brief update state of the cluster, which is caled for each finalized segment
 */
void
Cluster::updateClusterState()
{
    std::lock_guard<std::mutex> guard(m_segmentCounterLock);

    segmentCounter++;
    if(segmentCounter < allSegments.size()) {
        return;
    }

    // trigger next lerning phase, if already in phase 1
    if(mode == Cluster::LEARN_FORWARD_MODE)
    {
        mode = Cluster::LEARN_BACKWARD_MODE;
        startBackwardCycle();
        return;
    }

    // if a client is configured for the cluster, send a reponse that the learning was finished
    if(mode == Cluster::LEARN_BACKWARD_MODE
            && msgClient != nullptr)
    {
        Kitsunemimi::Hanami::LearnEnd_Message msg;
        uint8_t buffer[96*1024];
        const uint64_t size = msg.createBlob(buffer, 96*1024);
        if(size == 0) {
            return;
        }

        Kitsunemimi::ErrorContainer error;
        msgClient->sendStreamMessage(buffer, size, false, error);
    }

    // if a client is configured for the cluster, send a reponse that the request was finished
    if(mode == Cluster::NORMAL_MODE
            && msgClient != nullptr)
    {
        Kitsunemimi::Hanami::RequestEnd_Message msg;
        uint8_t buffer[96*1024];
        const uint64_t size = msg.createBlob(buffer, 96*1024);
        if(size == 0) {
            return;
        }

        Kitsunemimi::ErrorContainer error;
        msgClient->sendStreamMessage(buffer, size, false, error);
    }

    goToNextState(NEXT);
}

/**
 * @brief create a learn-task and add it to the task-queue
 *
 * @param inputData input-data
 * @param numberOfInputsPerCycle number of inputs per cycle
 * @param numberOfOuputsPerCycle number of outputs per cycle
 * @param numberOfCycle number of cycles
 *
 * @return task-uuid
 */
const std::string
Cluster::addImageLearnTask(float* inputData,
                           const uint64_t numberOfInputsPerCycle,
                           const uint64_t numberOfOuputsPerCycle,
                           const uint64_t numberOfCycle)
{
    // create new learn-task
    Task newTask;
    newTask.uuid = Kitsunemimi::Hanami::generateUuid();
    newTask.inputData = inputData;
    newTask.type = IMAGE_LEARN_TASK;
    newTask.progress.state = QUEUED_TASK_STATE;
    newTask.progress.queuedTimeStamp = std::chrono::system_clock::now();

    // fill metadata
    newTask.metaData.insert("number_of_cycles",
                            new DataValue(static_cast<long>(numberOfCycle)));
    newTask.metaData.insert("number_of_inputs_per_cycle",
                            new DataValue(static_cast<long>(numberOfInputsPerCycle)));
    newTask.metaData.insert("number_of_outputs_per_cycle",
                            new DataValue(static_cast<long>(numberOfOuputsPerCycle)));

    // add task to queue
    const std::string uuid = newTask.uuid.toString();
    m_taskHandleState->addTask(uuid, newTask);

    m_stateMachine->goToNextState(PROCESS_TASK);

    return uuid;
}

/**
 * @brief create a request-task and add it to the task-queue
 *
 * @param inputData input-data
 * @param numberOfInputsPerCycle number of inputs per cycle
 * @param numberOfOuputsPerCycle number of outputs per cycle
 * @param numberOfCycle number of cycles
 *
 * @return task-uuid
 */
const std::string
Cluster::addImageRequestTask(float* inputData,
                             const uint64_t numberOfInputsPerCycle,
                             const uint64_t numberOfOuputsPerCycle,
                             const uint64_t numberOfCycle)
{
    // create new request-task
    Task newTask;
    newTask.uuid = Kitsunemimi::Hanami::generateUuid();
    newTask.inputData = inputData;
    newTask.resultData = new DataArray();
    newTask.type = IMAGE_REQUEST_TASK;
    newTask.progress.state = QUEUED_TASK_STATE;
    newTask.progress.queuedTimeStamp = std::chrono::system_clock::now();

    // fill metadata
    newTask.metaData.insert("number_of_cycles",
                            new DataValue(static_cast<long>(numberOfCycle)));
    newTask.metaData.insert("number_of_inputs_per_cycle",
                            new DataValue(static_cast<long>(numberOfInputsPerCycle)));
    newTask.metaData.insert("number_of_outputs_per_cycle",
                            new DataValue(static_cast<long>(numberOfOuputsPerCycle)));

    // add task to queue
    const std::string uuid = newTask.uuid.toString();
    m_taskHandleState->addTask(uuid, newTask);

    m_stateMachine->goToNextState(PROCESS_TASK);

    return uuid;
}

/**
 * @brief create task to learn graph-data and add it to the task-queue
 *
 * @param inputData input-data
 * @param numberOfInputs number of inputs per cycle
 * @param numberOfCycle number of cycles
 *
 * @return task-uuid
 */
const std::string
Cluster::addGraphLearnTask(float* inputData,
                           const uint64_t numberOfInputs,
                           const uint64_t numberOfCycle)
{
    // create new learn-task
    Task newTask;
    newTask.uuid = Kitsunemimi::Hanami::generateUuid();
    newTask.inputData = inputData;
    newTask.type = GRAPH_LEARN_TASK;
    newTask.progress.state = QUEUED_TASK_STATE;
    newTask.progress.queuedTimeStamp = std::chrono::system_clock::now();

    // fill metadata
    newTask.metaData.insert("number_of_cycles",
                            new DataValue(static_cast<long>(numberOfCycle)));
    newTask.metaData.insert("number_of_inputs_per_cycle",
                            new DataValue(static_cast<long>(numberOfInputs)));

    // add task to queue
    const std::string uuid = newTask.uuid.toString();
    m_taskHandleState->addTask(uuid, newTask);

    m_stateMachine->goToNextState(PROCESS_TASK);

    return uuid;
}

/**
 * @brief create task to request graph-data and add it to the task-queue
 *
 * @param inputData input-data
 * @param numberOfInputs number of inputs per cycle
 * @param numberOfCycle number of cycles
 *
 * @return task-uuid
 */
const std::string
Cluster::addGraphRequestTask(float* inputData,
                             const uint64_t numberOfInputs,
                             const uint64_t numberOfCycle)
{
    // create new request-task
    Task newTask;
    newTask.uuid = Kitsunemimi::Hanami::generateUuid();
    newTask.inputData = inputData;
    newTask.resultData = new DataArray();
    newTask.type = GRAPH_REQUEST_TASK;
    newTask.progress.state = QUEUED_TASK_STATE;
    newTask.progress.queuedTimeStamp = std::chrono::system_clock::now();

    // fill metadata
    newTask.metaData.insert("number_of_cycles",
                            new DataValue(static_cast<long>(numberOfCycle)));
    newTask.metaData.insert("number_of_inputs_per_cycle",
                            new DataValue(static_cast<long>(numberOfInputs)));

    // add tasgetNextTaskk to queue
    const std::string uuid = newTask.uuid.toString();
    m_taskHandleState->addTask(uuid, newTask);

    m_stateMachine->goToNextState(PROCESS_TASK);

    return uuid;
}

/**
 * @brief create task to create a snapshot from a cluster and add it to the task-queue
 *
 * @param snapshotName name for the snapshot
 * @param userUuid uuid of the user, where the snapshot belongs to
 * @param projectUuid uuid of the project, where the snapshot belongs to
 *
 * @return task-uuid
 */
const std::string
Cluster::addClusterSnapshotSaveTask(const std::string &snapshotName,
                                    const std::string &userUuid,
                                    const std::string &projectUuid)
{
    // create new request-task
    Task newTask;
    newTask.uuid = Kitsunemimi::Hanami::generateUuid();
    newTask.type = CLUSTER_SNAPSHOT_SAVE_TASK;
    newTask.progress.state = QUEUED_TASK_STATE;
    newTask.progress.queuedTimeStamp = std::chrono::system_clock::now();

    // fill metadata
    newTask.metaData.insert("snapshot_name", new DataValue(snapshotName));
    newTask.metaData.insert("user_uuid", new DataValue(userUuid));
    newTask.metaData.insert("project_uuid", new DataValue(projectUuid));

    // add tasgetNextTaskk to queue
    const std::string uuid = newTask.uuid.toString();
    m_taskHandleState->addTask(uuid, newTask);

    m_stateMachine->goToNextState(PROCESS_TASK);

    return uuid;
}

/**
 * @brief create task to restore a cluster from a snapshot and add it to the task-queue
 *
 * @param snapshotUuid uuid of the snapshot
 * @param userUuid uuid of the user, where the snapshot belongs to
 * @param projectUuid uuid of the project, where the snapshot belongs to
 *
 * @return task-uuid
 */
const std::string
Cluster::addClusterSnapshotRestoreTask(const std::string &snapshotInfo,
                                       const std::string &userUuid,
                                       const std::string &projectUuid)
{
    // create new request-task
    Task newTask;
    newTask.uuid = Kitsunemimi::Hanami::generateUuid();
    newTask.type = CLUSTER_SNAPSHOT_RESTORE_TASK;
    newTask.progress.state = QUEUED_TASK_STATE;
    newTask.progress.queuedTimeStamp = std::chrono::system_clock::now();

    // fill metadata
    newTask.metaData.insert("snapshot_info", new DataValue(snapshotInfo));
    newTask.metaData.insert("user_uuid", new DataValue(userUuid));
    newTask.metaData.insert("project_uuid", new DataValue(projectUuid));

    // add tasgetNextTaskk to queue
    const std::string uuid = newTask.uuid.toString();
    m_taskHandleState->addTask(uuid, newTask);

    m_stateMachine->goToNextState(PROCESS_TASK);

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
    const std::string taskUuid = addImageRequestTask(inputData, numberOfInputes, 0, 1);
    segmentCounter = allSegments.size();
    updateClusterState();

    // wait until task is finished
    while(isFinish(taskUuid) == false) {
        usleep(10000);
    }

    // get result and clear backend
    // TODO: get result
    removeTask(taskUuid);

    return 0;
}

/**
 * @brief get actual task
 *
 * @return pointer to the actual task
 */
Task*
Cluster::getActualTask() const
{
    return m_taskHandleState->getActualTask();
}

/**
 * @brief get cycle of the actual task
 *
 * @return cycle of the actual task
 */
uint64_t
Cluster::getActualTaskCycle() const
{
    return m_taskHandleState->getActualTask()->actualCycle;
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
    return m_taskHandleState->getProgress(taskUuid);
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
    return m_taskHandleState->removeTask(taskUuid);
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
    return m_taskHandleState->isFinish(taskUuid);
}

/**
 * @brief NetworkCluster::setResultForActualCycle
 *
 * @param result
 */
void
Cluster::setResultForActualCycle(const uint32_t result)
{
    return m_taskHandleState->setResultForActualCycle(result);
}

/**
 * @brief switch statemachine of cluster to next state
 *
 * @param nextStateId id of the next state
 *
 * @return true, if statemachine switch was successful, else false
 */
bool
Cluster::goToNextState(const uint32_t nextStateId)
{
    return m_stateMachine->goToNextState(nextStateId);
}
