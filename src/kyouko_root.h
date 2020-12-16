/**
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
class ClientConnectionHandler;
class MonitoringConnectionHandler;

class KyoukoRoot
{

public:
    KyoukoRoot();
    ~KyoukoRoot();

    static KyoukoRoot* m_root;
    static Segment* m_segment;
    static ClientConnectionHandler* m_clientHandler;
    static MonitoringConnectionHandler* m_monitoringHandler;

    static Kitsunemimi::Kyouko::MonitoringBrickMessage monitoringBrickMessage;
    static Kitsunemimi::Kyouko::MonitoringProcessingTimes monitoringMetaMessage;

    bool start();
    void initBlossoms();
    bool initSakuraFiles();

    bool learn(const std::string &input,
               const std::string &should, std::string &);

private:
    NetworkManager* m_networkManager = nullptr;
    uint32_t m_serverId = 0;
};

#endif //KYOUKO_ROOT_OBJECT_H
