/**
 * @file        network_manager.h
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

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <common.h>
#include <random>
#include <libKitsunemimiCommon/threading/thread.h>

namespace Kitsunemimi {
class Barrier;
}

class ProcessingUnitHandler;

class NetworkManager
        : public Kitsunemimi::Thread
{

public:
    NetworkManager();

    void run();

private:
    bool initNetwork();

    ProcessingUnitHandler* m_processingUnitHandler = nullptr;
    Kitsunemimi::Barrier* m_phase1 = nullptr;
    Kitsunemimi::Barrier* m_phase2 = nullptr;
    Kitsunemimi::Barrier* m_phase3 = nullptr;

    uint32_t m_numberOfThreads = 0;
    std::string m_testMonitoring = "";

    float m_outBuffer[10];
    uint8_t m_outBufferPos = 0;

    std::vector<float> m_actualOutput;
    std::vector<float> m_should;
};

#endif // NETWORK_MANAGER_H
