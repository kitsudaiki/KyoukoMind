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

class InputSegment;
class OutputSegment;
class AbstractSegment;

struct ClusterMetaData
{
    kuuid uuid;

    float lerningValue = 0.0f;
    uint32_t cycleTime = 1000000;
    uint32_t numberOfInputSegments = 0;
    uint32_t numberOfOutputSegments = 0;
    uint32_t numberOfSegments = 0;

    uint8_t padding[196];
};

class NetworkCluster
{
public:
    NetworkCluster();

    Kitsunemimi::DataBuffer clusterData;

    ClusterMetaData* networkMetaData = nullptr;

    std::vector<InputSegment*> inputSegments;
    std::vector<OutputSegment*> outputSegments;
    std::vector<AbstractSegment*> allSegments;
    std::deque<AbstractSegment*> segmentQueue;

    const std::string initNewCluster(const JsonItem &parsedContent);

private:
    AbstractSegment* addInputSegment(const JsonItem &parsedContent);
    AbstractSegment* addOutputSegment(const JsonItem &parsedContent);
    AbstractSegment* addDynamicSegment(const JsonItem &parsedContent);
    bool prepareSegments(const JsonItem &parsedContent);
    uint32_t checkSegments(const JsonItem &parsedContent, const Position nextPos);

    void initSegmentPointer(const ClusterMetaData &metaData);
};

#endif // KYOUKOMIND_NETWORK_CLUSTER_H
