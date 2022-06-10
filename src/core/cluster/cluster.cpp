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

#include <core/cluster/states/task_handle_state.h>
#include <core/cluster/states/cycle_finish_state.h>
#include <core/cluster/states/graph_interpolation_state.h>
#include <core/cluster/states/graph_learn_backward_state.h>
#include <core/cluster/states/graph_learn_forward_state.h>
#include <core/cluster/states/image_identify_state.h>
#include <core/cluster/states/image_learn_backward_state.h>
#include <core/cluster/states/image_learn_forward_state.h>
#include <core/cluster/states/save_cluster_state.h>
#include <core/cluster/states/restore_cluster_state.h>

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



    initStatemachine();
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
Cluster::startForwardCycle()
{
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

    segmentCounter = 0;
    KyoukoRoot::m_segmentQueue->addSegmentListToQueue(allSegments);
}

/**
 * @brief NetworkCluster::updateClusterState
 */
void
Cluster::updateClusterState()
{
    std::lock_guard<std::mutex> guard(m_segmentCounterLock);

    segmentCounter++;
    if(segmentCounter < allSegments.size()) {
        return;
    }

    goToNextState(NEXT);
}

/**
 * @brief Cluster::initStatemachine
 */
void
Cluster::initStatemachine()
{
    m_stateMachine->createNewState(TASK_STATE,
                                   "Task-handling state");
    m_stateMachine->createNewState(LEARN_STATE,
                                   "Learn-State");
    m_stateMachine->createNewState(IMAGE_LEARN_STATE,
                                   "Image-learn state");
    m_stateMachine->createNewState(IMAGE_LEARN_FORWARD_STATE,
                                   "Image-learn state: forward-propagation");
    m_stateMachine->createNewState(IMAGE_LEARN_BACKWARD_STATE,
                                   "Image-learn state: backward-propagation");
    m_stateMachine->createNewState(IMAGE_LEARN_CYCLE_FINISH_STATE,
                                   "Image-learn state: finish-cycle");
    m_stateMachine->createNewState(GRAPH_LEARN_STATE,
                                   "Graph-learn state");
    m_stateMachine->createNewState(GRAPH_LEARN_FORWARD_STATE,
                                   "Graph-learn state: forward-propagation");
    m_stateMachine->createNewState(GRAPH_LEARN_BACKWARD_STATE,
                                   "Graph-learn state: backward-propagation");
    m_stateMachine->createNewState(GRAPH_LEARN_CYCLE_FINISH_STATE,
                                   "Graph-learn state: finish-cycle");
    m_stateMachine->createNewState(REQUEST_STATE,
                                   "Request-State");
    m_stateMachine->createNewState(IMAGE_REQUEST_STATE,
                                   "Image-request state");
    m_stateMachine->createNewState(IMAGE_REQUEST_FORWARD_STATE,
                                   "Image-request state: forward-propagation");
    m_stateMachine->createNewState(IMAGE_REQUEST_CYCLE_FINISH_STATE,
                                   "Image-request state: finish-cycle");
    m_stateMachine->createNewState(GRAPH_REQUEST_STATE,
                                   "Graph-request state");
    m_stateMachine->createNewState(GRAPH_REQUEST_FORWARD_STATE,
                                   "Graph-request state: forward-propagation");
    m_stateMachine->createNewState(GRAPH_REQUEST_CYCLE_FINISH_STATE,
                                   "Graph-request state: finish-cycle");
    m_stateMachine->createNewState(SNAPSHOT_STATE,
                                   "Snapshot state");
    m_stateMachine->createNewState(CLUSTER_SNAPSHOT_STATE,
                                   "Cluster-snapshot state");
    m_stateMachine->createNewState(CLUSTER_SNAPSHOT_SAVE_STATE,
                                   "Cluster-snapshot state: save");
    m_stateMachine->createNewState(CLUSTER_SNAPSHOT_RESTORE_STATE,
                                   "Cluster-snapshot state: restore");

    // add events to states
    m_stateMachine->addEventToState(TASK_STATE,
                                    m_taskHandleState);
    m_stateMachine->addEventToState(IMAGE_LEARN_FORWARD_STATE,
                                    new ImageLearnForward_State(this));
    m_stateMachine->addEventToState(IMAGE_LEARN_BACKWARD_STATE,
                                    new ImageLearnBackward_State(this));
    m_stateMachine->addEventToState(GRAPH_LEARN_FORWARD_STATE,
                                    new GraphLearnForward_State(this));
    m_stateMachine->addEventToState(GRAPH_LEARN_BACKWARD_STATE,
                                    new GraphLearnBackward_State(this));
    m_stateMachine->addEventToState(IMAGE_REQUEST_FORWARD_STATE,
                                    new ImageIdentify_State(this));
    m_stateMachine->addEventToState(GRAPH_REQUEST_FORWARD_STATE,
                                    new GraphInterpolation_State(this));
    m_stateMachine->addEventToState(IMAGE_LEARN_CYCLE_FINISH_STATE,
                                    new CycleFinish_State(this));
    m_stateMachine->addEventToState(GRAPH_LEARN_CYCLE_FINISH_STATE,
                                    new CycleFinish_State(this));
    m_stateMachine->addEventToState(IMAGE_REQUEST_CYCLE_FINISH_STATE,
                                    new CycleFinish_State(this));
    m_stateMachine->addEventToState(GRAPH_REQUEST_CYCLE_FINISH_STATE,
                                    new CycleFinish_State(this));
    m_stateMachine->addEventToState(CLUSTER_SNAPSHOT_SAVE_STATE,
                                    new SaveCluster_State(this));
    m_stateMachine->addEventToState(CLUSTER_SNAPSHOT_RESTORE_STATE,
                                    new RestoreCluster_State(this));

    // child states image learn
    m_stateMachine->addChildState(LEARN_STATE,       IMAGE_LEARN_STATE);
    m_stateMachine->addChildState(IMAGE_LEARN_STATE, IMAGE_LEARN_FORWARD_STATE);
    m_stateMachine->addChildState(IMAGE_LEARN_STATE, IMAGE_LEARN_BACKWARD_STATE);
    m_stateMachine->addChildState(IMAGE_LEARN_STATE, IMAGE_LEARN_CYCLE_FINISH_STATE);

    // child states graph learn
    m_stateMachine->addChildState(LEARN_STATE,       GRAPH_LEARN_STATE);
    m_stateMachine->addChildState(GRAPH_LEARN_STATE, GRAPH_LEARN_FORWARD_STATE);
    m_stateMachine->addChildState(GRAPH_LEARN_STATE, GRAPH_LEARN_BACKWARD_STATE);
    m_stateMachine->addChildState(GRAPH_LEARN_STATE, GRAPH_LEARN_CYCLE_FINISH_STATE);

    // child states image request
    m_stateMachine->addChildState(REQUEST_STATE,       IMAGE_REQUEST_STATE);
    m_stateMachine->addChildState(IMAGE_REQUEST_STATE, IMAGE_REQUEST_FORWARD_STATE);
    m_stateMachine->addChildState(IMAGE_REQUEST_STATE, IMAGE_REQUEST_CYCLE_FINISH_STATE);

    // child states graph request
    m_stateMachine->addChildState(REQUEST_STATE,       GRAPH_REQUEST_STATE);
    m_stateMachine->addChildState(GRAPH_REQUEST_STATE, GRAPH_REQUEST_FORWARD_STATE);
    m_stateMachine->addChildState(GRAPH_REQUEST_STATE, GRAPH_REQUEST_CYCLE_FINISH_STATE);

    // child states snapshot
    m_stateMachine->addChildState(SNAPSHOT_STATE,         CLUSTER_SNAPSHOT_STATE);
    m_stateMachine->addChildState(CLUSTER_SNAPSHOT_STATE, CLUSTER_SNAPSHOT_SAVE_STATE);
    m_stateMachine->addChildState(CLUSTER_SNAPSHOT_STATE, CLUSTER_SNAPSHOT_RESTORE_STATE);

    // set initial childs
    m_stateMachine->setInitialChildState(IMAGE_LEARN_STATE,   IMAGE_LEARN_FORWARD_STATE);
    m_stateMachine->setInitialChildState(GRAPH_LEARN_STATE,   GRAPH_LEARN_FORWARD_STATE);
    m_stateMachine->setInitialChildState(IMAGE_REQUEST_STATE, IMAGE_REQUEST_FORWARD_STATE);
    m_stateMachine->setInitialChildState(GRAPH_REQUEST_STATE, GRAPH_REQUEST_FORWARD_STATE);

    // transtions learn init
    m_stateMachine->addTransition(TASK_STATE,  LEARN, LEARN_STATE);
    m_stateMachine->addTransition(LEARN_STATE, IMAGE, IMAGE_LEARN_STATE);
    m_stateMachine->addTransition(LEARN_STATE, GRAPH, GRAPH_LEARN_STATE);

    // transitions request init
    m_stateMachine->addTransition(TASK_STATE,    REQUEST, REQUEST_STATE);
    m_stateMachine->addTransition(REQUEST_STATE, IMAGE,   IMAGE_REQUEST_STATE);
    m_stateMachine->addTransition(REQUEST_STATE, GRAPH,   GRAPH_REQUEST_STATE);

    // transitions snapshot init
    m_stateMachine->addTransition(TASK_STATE,             SNAPSHOT, SNAPSHOT_STATE);
    m_stateMachine->addTransition(SNAPSHOT_STATE,         CLUSTER,  CLUSTER_SNAPSHOT_STATE);
    m_stateMachine->addTransition(CLUSTER_SNAPSHOT_STATE, SAVE,     CLUSTER_SNAPSHOT_SAVE_STATE);
    m_stateMachine->addTransition(CLUSTER_SNAPSHOT_STATE, RESTORE,  CLUSTER_SNAPSHOT_RESTORE_STATE);

    // trainsition learn-internal
    m_stateMachine->addTransition(IMAGE_LEARN_FORWARD_STATE,      NEXT, IMAGE_LEARN_BACKWARD_STATE     );
    m_stateMachine->addTransition(IMAGE_LEARN_BACKWARD_STATE,     NEXT, IMAGE_LEARN_CYCLE_FINISH_STATE );
    m_stateMachine->addTransition(IMAGE_LEARN_CYCLE_FINISH_STATE, NEXT, IMAGE_LEARN_FORWARD_STATE      );
    m_stateMachine->addTransition(GRAPH_LEARN_FORWARD_STATE,      NEXT, GRAPH_LEARN_BACKWARD_STATE     );
    m_stateMachine->addTransition(GRAPH_LEARN_BACKWARD_STATE,     NEXT, GRAPH_LEARN_CYCLE_FINISH_STATE );
    m_stateMachine->addTransition(GRAPH_LEARN_CYCLE_FINISH_STATE, NEXT, GRAPH_LEARN_FORWARD_STATE      );

    // trainsition request-internal
    m_stateMachine->addTransition(IMAGE_REQUEST_FORWARD_STATE,      NEXT, IMAGE_REQUEST_CYCLE_FINISH_STATE );
    m_stateMachine->addTransition(IMAGE_REQUEST_CYCLE_FINISH_STATE, NEXT, IMAGE_REQUEST_FORWARD_STATE      );
    m_stateMachine->addTransition(GRAPH_REQUEST_FORWARD_STATE,      NEXT, GRAPH_REQUEST_CYCLE_FINISH_STATE );
    m_stateMachine->addTransition(GRAPH_REQUEST_CYCLE_FINISH_STATE, NEXT, GRAPH_REQUEST_FORWARD_STATE      );

    // transition finish back to task-state
    m_stateMachine->addTransition(LEARN_STATE,                    FINISH_TASK, TASK_STATE);
    m_stateMachine->addTransition(REQUEST_STATE,                  FINISH_TASK, TASK_STATE);
    m_stateMachine->addTransition(SNAPSHOT_STATE,                 FINISH_TASK, TASK_STATE);
    m_stateMachine->addTransition(CLUSTER_SNAPSHOT_SAVE_STATE,    FINISH_TASK, TASK_STATE);
    m_stateMachine->addTransition(CLUSTER_SNAPSHOT_RESTORE_STATE, FINISH_TASK, TASK_STATE);

    // special transition to tigger the task-state again
    m_stateMachine->addTransition(TASK_STATE, PROCESS_TASK, TASK_STATE);

    // set initial state for the state-machine
    m_stateMachine->setCurrentState(TASK_STATE);
}

/**
 * @brief create a learn-task and add it to the task-queue
 *
 * @param inputData input-data
 * @param numberOfInputsPerCycle number of inputs, which numberOfInputsPerCyclebelongs to one cycle
 * @param numberOfOuputsPerCycle number of outputs, which belongs to one cycle
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
 * @param numberOfInputsPerCycle number of inputs, which belongs to one cycle
 * @param numberOfOuputsPerCycle number of outputs of the dataset-only to calculate correct offset
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
 * @brief Cluster::addGraphLearnTask
 * @param inputData
 * @param numberOfValues
 * @param numberOfInputs
 * @param numberOfCycle
 * @return
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
 * @brief Cluster::addGraphRequestTask
 * @param inputData
 * @param numberOfValues
 * @param numberOfInputs
 * @param numberOfCycle
 * @return
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
 * @brief Cluster::addClusterSnapshotSaveTask
 * @param snapshotName
 * @param userUuid
 * @param projectUuid
 * @return
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
 * @brief Cluster::addClusterSnapshotRestoreTask
 * @param snapshotUuid
 * @param userUuid
 * @param projectUuid
 * @return
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
 * @brief Cluster::getActualTask
 * @return
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
 * @brief Cluster::goToNextState
 * @param nextStateId
 * @return
 */
bool
Cluster::goToNextState(const uint32_t nextStateId)
{
    return m_stateMachine->goToNextState(nextStateId);
}
