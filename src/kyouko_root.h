/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef KYOUKO_ROOT_OBJECT_H
#define KYOUKO_ROOT_OBJECT_H

#include <common.h>

namespace Kitsunemimi {
namespace Project {
class Session;
class SessionController;
}
}

namespace KyoukoMind
{
class NetworkManager;
class DummyInput;
class NetworkSegment;
class Brick;

class KyoukoRoot
{

public:
    KyoukoRoot();
    ~KyoukoRoot();

    static KyoukoMind::NetworkSegment* m_segment;
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
    DummyInput* m_dummyInput = nullptr;
};

} // namespace KyoukoMind

#endif //KYOUKO_ROOT_OBJECT_H
