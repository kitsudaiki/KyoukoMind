/**
 *  @file    kyouko_network.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef KYOUKO_ROOT_OBJECT_H
#define KYOUKO_ROOT_OBJECT_H

#include <common.h>

#include <core/global_values_handler.h>
#include <core/brick_queue.h>

namespace Kitsunemimi {
namespace Project {
class Session;
class SessionController;
}
}

namespace KyoukoMind
{
class NetworkManager;
class BrickQueue;
struct NetworkSegment;

class RootObject
{

public:
    RootObject();
    ~RootObject();

    static KyoukoMind::NetworkSegment* m_segment;
    static KyoukoMind::BrickQueue* m_queue;
    static KyoukoMind::GlobalValuesHandler* m_globalValuesHandler;

    static Kitsunemimi::Project::Session* m_clientSession;
    static Kitsunemimi::Project::Session* m_controlSession;
    static Kitsunemimi::Project::Session* m_monitoringSession;

    static std::map<uint32_t, Brick*>* m_inputBricks;

    bool start();
    bool initServer();

    const std::string convertToObj();

private:
    Kitsunemimi::Project::SessionController* m_sessionController = nullptr;
    NetworkManager* m_networkManager = nullptr;
    uint32_t m_serverId = 0;
};

} // namespace KyoukoMind

#endif //KYOUKO_ROOT_OBJECT_H
