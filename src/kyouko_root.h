/**
 * @file        kyouko_root.h
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

#ifndef KYOUKO_ROOT_OBJECT_H
#define KYOUKO_ROOT_OBJECT_H

#include <common.h>

class NetworkManager;
struct Segment;
struct NetworkCluster;
class InputOutputProcessing;
class ClientConnectionHandler;
class MonitoringConnectionHandler;

class KyoukoRoot
{

public:
    KyoukoRoot();
    ~KyoukoRoot();

    static KyoukoRoot* m_root;
    static NetworkCluster* m_networkCluster;
    static bool m_freezeState;
    static ClientConnectionHandler* m_clientHandler;
    static MonitoringConnectionHandler* m_monitoringHandler;
    static InputOutputProcessing* m_ioHandler;

    bool start();
    bool initializeSakuraFiles();

    bool learnStep();
    void executeStep();
    void finishStep();
    void learnTestData();

    NetworkManager* m_networkManager = nullptr;

private:
    uint32_t m_serverId = 0;

    float m_inputBuffer[800];
};

#endif //KYOUKO_ROOT_OBJECT_H
