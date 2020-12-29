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
#include "init_meta_data.h"

class BrickQueue;
class Segment;

class NetworkInitializer
{

public:
    NetworkInitializer();

    bool createNewNetwork(const std::string &fileContent);

private:
    std::vector<std::vector<InitMetaDataEntry>> m_netMetaStructure;

    void connectAllBricks(Segment &segment);
    void addBricks(Segment &segment);

    uint32_t getNumberOfNodeBricks();

    std::pair<uint32_t, uint32_t> getNext(const uint32_t x,
                                          const uint32_t y,
                                          const uint8_t side);

    uint32_t getDistantToNextNodeBrick(const uint32_t x,
                                       const uint32_t y,
                                       const uint8_t side);

};

#endif // NETWORK_INITIALIZER_H
