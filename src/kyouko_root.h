﻿/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef KYOUKO_ROOT_OBJECT_H
#define KYOUKO_ROOT_OBJECT_H

#include <common.h>
#include <libKitsunemimiKyoukoCommon/communication_structs/monitoring_contianer.h>

class NetworkManager;
class Segment;
class Brick;

struct arrayPos {
    uint32_t position = 0;
    uint32_t range = 0;

    arrayPos(const uint32_t position, const uint32_t range)
    {
        this->position = position;
        this->range = range;
    }
};

class KyoukoRoot
{

public:
    KyoukoRoot();
    ~KyoukoRoot();

    static KyoukoRoot* m_root;
    static Segment* m_segment;
    static std::map<uint32_t, Brick*>* m_inputBricks;

    static Kitsunemimi::Kyouko::MonitoringBrickMessage monitoringBrickMessage;
    static Kitsunemimi::Kyouko::MonitoringMetaMessage monitoringMetaMessage;

    static std::map<uint32_t, arrayPos> registeredInputs;
    static std::map<uint32_t, arrayPos> registeredOutputs;

    bool start();
    bool initSakuraFiles();

    bool learn(const std::string &input,
               const std::string &should,
               std::string &errorMessage);

private:
    NetworkManager* m_networkManager = nullptr;
    uint32_t m_serverId = 0;
    const std::string getTestConfig();
};

#endif //KYOUKO_ROOT_OBJECT_H
