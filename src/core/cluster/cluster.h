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

namespace Kitsunemimi {
class EventQueue;
class Statemachine;
namespace Hanami {
class HanamiMessagingClient;
}
}

class Cluster
{
public:
    Cluster();
    ~Cluster();

    enum ClusterProcessingMode
    {
        NORMAL_MODE = 0,
        LEARN_FORWARD_MODE = 1,
        LEARN_BACKWARD_MODE = 2,
    };

    // cluster-data
    Kitsunemimi::DataBuffer clusterData;
    ClusterMetaData* networkMetaData = nullptr;
    ClusterSettings* networkSettings = nullptr;
    std::map<std::string, InputSegment*> inputSegments;
    std::map<std::string, OutputSegment*> outputSegments;
    std::vector<AbstractSegment*> allSegments;

    const std::string getUuid();
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
    bool setClusterState(const std::string &newState);

    uint32_t segmentCounter = 0;
    ClusterProcessingMode mode = NORMAL_MODE;
    Kitsunemimi::Hanami::HanamiMessagingClient* msgClient = nullptr;

private:
    Kitsunemimi::Statemachine* m_stateMachine = nullptr;
    TaskHandle_State* m_taskHandleState = nullptr;
    std::mutex m_segmentCounterLock;
};

#endif // KYOUKOMIND_CLUSTER_INTERFACE_H
