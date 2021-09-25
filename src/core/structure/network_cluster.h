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
#include <core/objects/cluster_meta.h>

class InputSegment;
class OutputSegment;
class AbstractSegment;

class ClusterInterface;

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

    const std::string initNewCluster(const JsonItem &parsedContent, ClusterInterface *interface);

private:
    const std::string getName();
    bool setName(const std::string newName);
    void initSegmentPointer(const ClusterMetaData &metaData,
                            const ClusterSettings &settings);

    AbstractSegment* addInputSegment(const JsonItem &parsedContent);
    AbstractSegment* addOutputSegment(const JsonItem &parsedContent);
    AbstractSegment* addDynamicSegment(const JsonItem &parsedContent);
};

#endif // KYOUKOMIND_NETWORK_CLUSTER_H
