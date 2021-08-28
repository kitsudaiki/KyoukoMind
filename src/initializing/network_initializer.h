/**
 * @file        network_initializer.h
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

#ifndef NETWORK_INITIALIZER_H
#define NETWORK_INITIALIZER_H

#include <common.h>

struct Brick;
struct NetworkCluster;
class BrickInitializer;

class ClusterInitializer
{
public:
    ClusterInitializer();

    bool initNetwork(const std::string &filePath);
    bool createNewNetwork(JsonItem &parsedContent);

private:
    BrickInitializer* m_brickInitializer = nullptr;

    void addInputSegment(JsonItem &parsedContent, NetworkCluster* cluster);
    void addOutputSegment(JsonItem &parsedContent, NetworkCluster* cluster);
    void addDynamicSegment(JsonItem &parsedContent, NetworkCluster* cluster);
    Position getNeighborPos(const Position sourcePos, const uint8_t side);
    bool prepareSegments(JsonItem &parsedContent);
    uint32_t checkSegments(JsonItem &parsedContent, const Position nextPos);
};

#endif // NETWORK_INITIALIZER_H
