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
#include <core/cluster/task.h>
#include <core/cluster/cluster_meta.h>

class AbstractSegment;
class InputSegment;
class OutputSegment;
class AbstractSegment;

class TaskHandle_State;
class CycleFinish_State;
class GraphInterpolation_State;
class GraphLearnBackward_State;
class GraphLearnForward_State;
class ImageIdentify_State;
class ImageLearnBackward_State;
class ImageLearnForward_State;

namespace Kitsunemimi {
class EventQueue;
class Statemachine;
}

class Cluster
{
public:
    Cluster();
    ~Cluster();

    enum ClusterStates
    {
        TASK_STATE = 0,
        LEARN_STATE = 1,
            IMAGE_LEARN_STATE = 2,
                IMAGE_LEARN_FORWARD_STATE = 3,
                IMAGE_LEARN_BACKWARD_STATE = 4,
                IMAGE_LEARN_CYCLE_FINISH_STATE = 5,
            GRAPH_LEARN_STATE = 6,
                GRAPH_LEARN_FORWARD_STATE = 7,
                GRAPH_LEARN_BACKWARD_STATE = 8,
                GRAPH_LEARN_CYCLE_FINISH_STATE = 9,
        REQUEST_STATE = 10,
            IMAGE_REQUEST_STATE = 11,
                IMAGE_REQUEST_FORWARD_STATE = 12,
                IMAGE_REQUEST_CYCLE_FINISH_STATE = 13,
            GRAPH_REQUEST_STATE = 14,
                GRAPH_REQUEST_FORWARD_STATE = 15,
                GRAPH_REQUEST_CYCLE_FINISH_STATE = 16,
        SNAPSHOT_STATE = 17,
            CLUSTER_SNAPSHOT_STATE = 18,
                CLUSTER_SNAPSHOT_SAVE_STATE = 19,
                CLUSTER_SNAPSHOT_RESTORE_STATE = 20,
    };

    enum ClusterTransitions
    {
        LEARN = 100,
        REQUEST = 101,
        SNAPSHOT = 102,
        IMAGE = 103,
        GRAPH = 104,
        CLUSTER = 105,
        SAVE = 106,
        RESTORE = 107,
        NEXT = 108,
        FINISH_TASK = 109,
        PROCESS_TASK = 110,
    };

    enum ClusterMode
    {
        NORMAL_MODE = 0,
        LEARN_FORWARD_MODE = 1,
        LEARN_BACKWARD_MODE = 2,
    };

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
    const std::string addImageLearnTask(float* inputData,
                                        const uint64_t numberOfInputsPerCycle,
                                        const uint64_t numberOfOuputsPerCycle,
                                        const uint64_t numberOfCycle);
    const std::string addImageRequestTask(float* inputData,
                                          const uint64_t numberOfInputsPerCycle,
                                          const uint64_t numberOfOuputsPerCycle,
                                          const uint64_t numberOfCycle);
    const std::string addGraphLearnTask(float* inputData,
                                        const uint64_t numberOfInputs,
                                        const uint64_t numberOfCycle);
    const std::string addGraphRequestTask(float* inputData,
                                          const uint64_t numberOfInputs,
                                          const uint64_t numberOfCycle);
    const std::string addClusterSnapshotSaveTask(const std::string &snapshotName,
                                                 const std::string &userUuid,
                                                 const std::string &projectUuid);
    const std::string addClusterSnapshotRestoreTask(const std::string &snapshotInfo,
                                                    const std::string &userUuid,
                                                    const std::string &projectUuid);

    uint32_t request(float* inputData, const uint64_t numberOfInputes);

    // tasks
    Task* getActualTask() const;
    uint64_t getActualTaskCycle() const;
    const TaskProgress getProgress(const std::string &taskUuid);
    bool removeTask(const std::string &taskUuid);
    bool isFinish(const std::string &taskUuid);
    void setResultForActualCycle(const uint32_t result);

    bool goToNextState(const uint32_t nextStateId);
    void startForwardCycle();
    void startBackwardCycle();

    uint32_t segmentCounter = 0;
    ClusterMode mode = NORMAL_MODE;

private:
    Kitsunemimi::Statemachine* m_stateMachine = nullptr;
    TaskHandle_State* m_taskHandleState = nullptr;
    std::mutex m_segmentCounterLock;

    void initStatemachine();
};

#endif // KYOUKOMIND_CLUSTER_INTERFACE_H
