/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef KYOUKO_ROOT_OBJECT_H
#define KYOUKO_ROOT_OBJECT_H

#include <common.h>

class NetworkManager;
class Segment;
class Brick;
class EventProcessing;

class KyoukoRoot
{

public:
    KyoukoRoot();
    ~KyoukoRoot();

    static Segment* m_segment;
    static std::map<uint32_t, Brick*>* m_inputBricks;

    bool start();
    bool initServer();

private:
    NetworkManager* m_networkManager = nullptr;
    uint32_t m_serverId = 0;
    const std::string getTestConfig();
};

#endif //KYOUKO_ROOT_OBJECT_H
