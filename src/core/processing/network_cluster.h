/**
 * @file        network_cluster.h
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

#ifndef KYOUKOMIND_NETWORK_CLUSTER_H
#define KYOUKOMIND_NETWORK_CLUSTER_H

#include <common.h>
#include <core/objects/task.h>
#include <core/objects/cluster_meta.h>

class InputSegment;
class OutputSegment;
class AbstractSegment;

class TaskQueue;

enum ClusterMode
{
    NORMAL_MODE = 0,
    LEARN_FORWARD_MODE = 1,
    LEARN_BACKWARD_MODE = 2
};

class NetworkCluster
{
public:
    NetworkCluster();

    Kitsunemimi::DataBuffer clusterData;

    ClusterMetaData* networkMetaData = nullptr;
    ClusterSettings* networkSettings = nullptr;

    std::vector<InputSegment*> inputSegments;
    std::vector<OutputSegment*> outputSegments;
    std::vector<AbstractSegment*> allSegments;

    const std::string initNewCluster(const JsonItem &parsedContent);

    ClusterMode mode = NORMAL_MODE;
    uint32_t segmentCounter = 0;

    const std::string addLearnTask(float* data,
                                   const uint64_t numberOfInputsPerCycle,
                                   const uint64_t numberOfOuputsPerCycle,
                                   const uint64_t numberOfCycle);
    const std::string addRequestTask(float* inputData,
                                     const uint64_t numberOfInputsPerCycle,
                                     const uint64_t numberOfCycle);
    uint64_t getActualTaskCycle();
    const TaskProgress getProgress(const std::string &taskUuid);
    const uint32_t* getResultData(const std::string &taskUuid);
    bool isFinish(const std::string &taskUuid);
    void setResultForActualCycle(const uint32_t result);

    void updateClusterState();

private:
    AbstractSegment* addInputSegment(const JsonItem &parsedContent);
    AbstractSegment* addOutputSegment(const JsonItem &parsedContent);
    AbstractSegment* addDynamicSegment(const JsonItem &parsedContent);

    TaskQueue* m_taskQueue = nullptr;
    std::mutex m_task_mutex;

    const std::string getName();
    bool setName(const std::string newName);

    const std::string prepareDirection(const JsonItem &currentSegment,
                                       const JsonItem &segments,
                                       const uint32_t foundNext,
                                       const uint8_t side);
    long getNeighborBorderSize(const JsonItem &currentSegment,
                               const JsonItem &segments,
                               const uint32_t foundNext);
    bool prepareSingleSegment(std::deque<uint32_t> &segmentQueue,
                              const JsonItem &segments,
                              JsonItem &parsedSegments);
    bool prepareSegments(const JsonItem &parsedContent);
    uint32_t checkNextPosition(const JsonItem &segments, const Position nextPos);
    Position convertPosition(const JsonItem &parsedContent);

    void initSegmentPointer(const ClusterMetaData &metaData,
                            const ClusterSettings &settings);

    void startForwardLearnCycle();
    void startBackwardLearnCycle();
};

#endif // KYOUKOMIND_NETWORK_CLUSTER_H
