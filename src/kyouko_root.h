/**
 * @file        kyouko_root.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2018 Tobias Anker
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
#include <core/objects/monitoring_container.h>

class NetworkManager;
class Segment;
class Brick;
class ClientConnectionHandler;
class MonitoringConnectionHandler;

class KyoukoRoot
{

public:
    KyoukoRoot();
    ~KyoukoRoot();

    static KyoukoRoot* m_root;
    static Segment* m_segment;
    static bool m_freezeState;
    static ClientConnectionHandler* m_clientHandler;
    static MonitoringConnectionHandler* m_monitoringHandler;

    static MonitoringBrickMessage monitoringBrickMessage;
    static MonitoringProcessingTimes monitoringMetaMessage;

    bool start();
    void initBlossoms();
    bool initSakuraFiles();

    bool learn(const std::string &input,
               const std::string &should, std::string &);

    NetworkManager* m_networkManager = nullptr;

private:
    uint32_t m_serverId = 0;
};

#endif //KYOUKO_ROOT_OBJECT_H
