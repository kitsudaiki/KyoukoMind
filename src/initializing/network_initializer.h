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

class BrickQueue;
class Segment;

namespace Kitsunemimi {
namespace Ai {
struct AiBaseMeta;
}
}

class NetworkInitializer
{

public:
    NetworkInitializer();

    bool createNewNetwork(const std::string &fileContent);

private:
    void addBricks(Segment &segment,
                   const Kitsunemimi::Ai::AiBaseMeta &metaBase);

    bool createAxons(Segment &segment);
};

#endif // NETWORK_INITIALIZER_H
