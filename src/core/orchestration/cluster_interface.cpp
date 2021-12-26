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

#include "cluster_interface.h"

#include <core/orchestration/segments/dynamic_segment.h>
#include <core/orchestration/segments/input_segment.h>
#include <core/orchestration/segments/output_segment.h>

#include <core/orchestration/task_queue.h>
#include <core/orchestration/network_cluster.h>
#include <core/processing/segment_queue.h>
#include <core/processing/cpu/output_segment/processing.h>

#include <libKitsunemimiCommon/logger.h>
#include <kyouko_root.h>

ClusterInterface::ClusterInterface()
{
    m_cluster = new NetworkCluster;
    m_taskQueue = new TaskQueue();
}

bool
ClusterInterface::initNewCluster(const JsonItem &parsedContent,
                                 const std::string &uuid)
{
    return m_cluster->initNewCluster(parsedContent, uuid, this);
}

/**
 * @brief NetworkCluster::startNewCycle
 */
void
ClusterInterface::startForwardLearnCycle()
{
    OutputNode* outputs = m_cluster->outputSegments[0]->outputs;

    Task* actualTask = m_taskQueue->actualTask;
    const uint64_t offsetInput = actualTask->numberOfInputsPerCycle * actualTask->actualCycle;
    const uint64_t offsetLabels = actualTask->numberOfOuputsPerCycle * actualTask->actualCycle;

    // set cluster mode
    if(actualTask->type == LEARN_TASK) {
        m_mode = LEARN_FORWARD_MODE;
    }

    InputNode* inputNodes = m_cluster->inputSegments[0]->inputs;
    for(uint64_t i = 0; i < actualTask->numberOfInputsPerCycle; i++) {
        inputNodes[i].weight = actualTask->inputData[offsetInput + i];
    }

    for(uint64_t i = 0; i < actualTask->numberOfOuputsPerCycle; i++) {
        outputs[i].shouldValue = actualTask->labels[offsetLabels + i];
    }

    // set ready-states of all neighbors of all segments
    for(AbstractSegment* segment: m_cluster->allSegments)
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

    KyoukoRoot::m_segmentQueue->addSegmentListToQueue(m_cluster->allSegments);
}

/**
 * @brief NetworkCluster::startBackwardLearnCycle
 */
void
ClusterInterface::startBackwardLearnCycle()
{
    // set cluster mode
    m_mode = LEARN_BACKWARD_MODE;

    // set ready-states of all neighbors of all segments
    for(AbstractSegment* segment: m_cluster->allSegments)
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

    KyoukoRoot::m_segmentQueue->addSegmentListToQueue(m_cluster->allSegments);
}

/**
 * @brief NetworkCluster::updateClusterState
 */
void
ClusterInterface::updateClusterState()
{
    m_segmentCounter++;
    if(m_segmentCounter < m_cluster->allSegments.size()) {
        return;
    }

    m_segmentCounter = 0;

    if(m_mode == LEARN_FORWARD_MODE)
    {
        const float error = calcTotalError(m_cluster->outputSegments[0]);
        if(error > 0.05f) // TODO: make configurable
        {
            startBackwardLearnCycle();
            return;
        }
    }

    std::lock_guard<std::mutex> guard(m_task_mutex);

    m_mode = NORMAL_MODE;

    if(m_taskQueue->actualTask == nullptr)
    {
        if(m_taskQueue->getNextTask()) {
            startForwardLearnCycle();
        }
    }
    else
    {
        m_taskQueue->actualTask->actualCycle++;
        const float actualF = static_cast<float>(m_taskQueue->actualTask->actualCycle);
        const float shouldF = static_cast<float>(m_taskQueue->actualTask->numberOfCycle);
        m_taskQueue->actualTask->progress.percentageFinished = actualF / shouldF;

        if(m_taskQueue->actualTask->actualCycle == m_taskQueue->actualTask->numberOfCycle)
        {
            m_taskQueue->finishTask();
            if(m_taskQueue->getNextTask()) {
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
 * @brief ClusterInterface::getMode
 * @return
 */
ClusterMode
ClusterInterface::getMode() const
{
    return m_mode;
}

/**
 * @brief NetworkCluster::addLearnTask
 * @param data
 * @param numberOfInputsPerCycle
 * @param numberOfOuputsPerCycle
 * @param numberOfCycle
 * @return
 */
const std::string
ClusterInterface::addLearnTask(float* inputData,
                               float* labels,
                               const uint64_t numberOfInputsPerCycle,
                               const uint64_t numberOfOuputsPerCycle,
                               const uint64_t numberOfCycle)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);
    return m_taskQueue->addLearnTask(inputData,
                                     labels,
                                     numberOfInputsPerCycle,
                                     numberOfOuputsPerCycle,
                                     numberOfCycle);
}

/**
 * @brief NetworkCluster::addRequestTask
 * @param inputData
 * @param numberOfInputsPerCycle
 * @param numberOfCycle
 * @return
 */
const std::string
ClusterInterface::addRequestTask(float* inputData,
                                 const uint64_t numberOfInputsPerCycle,
                                 const uint64_t numberOfCycle)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);
    return m_taskQueue->addRequestTask(inputData,
                                       numberOfInputsPerCycle,
                                       numberOfCycle);
}

/**
 * @brief ClusterInterface::request
 * @param inputData
 * @param numberOfInputsPerCycle
 * @return
 */
uint32_t
ClusterInterface::request(float* inputData,
                          const uint64_t numberOfInputes)
{

    const std::string taskUuid = addRequestTask(inputData, numberOfInputes, 1);
    m_segmentCounter = getNumberOfSegments();
    updateClusterState();
    // wait until task is finished
    while(isFinish(taskUuid) == false) {
        usleep(10000);
    }

    const uint32_t result = getResultData(taskUuid)[0];
    return result;
}

/**
 * @brief NetworkCluster::getActualTaskCycle
 * @return
 */
uint64_t
ClusterInterface::getActualTaskCycle()
{
    std::lock_guard<std::mutex> guard(m_task_mutex);
    return m_taskQueue->actualTask->actualCycle;
}

/**
 * @brief NetworkCluster::getProgress
 * @param taskUuid
 * @return
 */
const TaskProgress
ClusterInterface::getProgress(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);
    return m_taskQueue->getProgress(taskUuid);
}

/**
 * @brief NetworkCluster::getResultData
 * @param taskUuid
 * @return
 */
const uint32_t*
ClusterInterface::getResultData(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);
    return m_taskQueue->getResultData(taskUuid);
}

/**
 * @brief ClusterInterface::removeResultData
 * @param taskUuid
 * @return
 */
bool
ClusterInterface::removeResultData(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);
    return m_taskQueue->removeTask(taskUuid);
}

/**
 * @brief NetworkCluster::isFinish
 * @param taskUuid
 * @return
 */
bool
ClusterInterface::isFinish(const std::string &taskUuid)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);
    return m_taskQueue->isFinish(taskUuid);
}

/**
 * @brief NetworkCluster::setResultForActualCycle
 * @param result
 */
void
ClusterInterface::setResultForActualCycle(const uint32_t result)
{
    std::lock_guard<std::mutex> guard(m_task_mutex);
    m_taskQueue->actualTask->resultData[m_taskQueue->actualTask->actualCycle] = result;
}

/**
 * @brief ClusterInterface::getNumberOfSegments
 * @return
 */
uint64_t
ClusterInterface::getNumberOfSegments() const
{
    return m_cluster->allSegments.size();
}

/**
 * @brief ClusterInterface::getSegment
 * @param id
 * @return
 */
AbstractSegment*
ClusterInterface::getSegment(const uint64_t id) const
{
    return m_cluster->allSegments.at(id);
}
