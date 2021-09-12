/**
 * @file        cluster_initializer.h
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

#ifndef KYOUKOMIND_CLUSTER_INITIALIZER_H
#define KYOUKOMIND_CLUSTER_INITIALIZER_H

#include <common.h>

struct Brick;
struct NetworkCluster;
class BrickInitializer;
class AbstractSegment;

class ClusterInitializer
{
public:
    ClusterInitializer();

    const std::string initCluster(const std::string &filePath);
    const std::string createNewNetwork(const JsonItem &parsedContent);

private:
    BrickInitializer* m_brickInitializer = nullptr;

    AbstractSegment* addInputSegment(const JsonItem &parsedContent, NetworkCluster* cluster);
    AbstractSegment* addOutputSegment(const JsonItem &parsedContent, NetworkCluster* cluster);
    AbstractSegment* addDynamicSegment(const JsonItem &parsedContent, NetworkCluster* cluster);
    bool prepareSegments(const JsonItem &parsedContent);
    uint32_t checkSegments(const JsonItem &parsedContent, const Position nextPos);
};

#endif // KYOUKOMIND_CLUSTER_INITIALIZER_H
