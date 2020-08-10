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

class NetworkManager;
class Segment;
class GpuProcessingUnit;
class Brick;
class EventProcessing;

class KyoukoRoot
{

public:
    KyoukoRoot();
    ~KyoukoRoot();

    static Segment* m_segment;
    static EventProcessing* m_eventProcessing;

    static Kitsunemimi::Project::Session* m_clientSession;
    static Kitsunemimi::Project::Session* m_controlSession;
    static Kitsunemimi::Project::Session* m_monitoringSession;

    static std::map<uint32_t, Brick*>* m_inputBricks;

    bool start();
    bool initServer();

private:
    Kitsunemimi::Project::SessionController* m_sessionController = nullptr;
    NetworkManager* m_networkManager = nullptr;
    uint32_t m_serverId = 0;
};

#endif //KYOUKO_ROOT_OBJECT_H
