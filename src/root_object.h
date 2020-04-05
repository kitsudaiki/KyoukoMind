/**
 *  @file    kyouko_network.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef KYOUKO_ROOT_OBJECT_H
#define KYOUKO_ROOT_OBJECT_H

#include <common.h>

#include <core/brick_handler.h>
#include <core/global_values_handler.h>

namespace Kitsunemimi {
namespace Project {
class Session;
class SessionController;
}
}

namespace KyoukoMind
{
class NetworkManager;
class BrickHandler;

class RootObject
{

public:
    RootObject();
    ~RootObject();

    static KyoukoMind::BrickHandler* m_brickHandler;
    static KyoukoMind::GlobalValuesHandler* m_globalValuesHandler;

    static Kitsunemimi::Project::Session* m_clientSession;
    static Kitsunemimi::Project::Session* m_monitoringSession;

    bool start();
    bool initServer();

private:
    Kitsunemimi::Project::SessionController* m_sessionController = nullptr;
    NetworkManager* m_networkManager = nullptr;
    uint32_t m_serverId = 0;

};

} // namespace KyoukoMind

#endif //KYOUKO_ROOT_OBJECT_H
