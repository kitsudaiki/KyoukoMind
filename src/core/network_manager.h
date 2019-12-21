/**
 *  @file    network_manager.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

namespace KyoukoMind
{

class Config;
class BrickHandler;
class ProcessingUnitHandler;
class MessageController;
class ConnectionTrigger;

class NetworkManager : public Kitsunemimi::Common::Thread
{

public:
    NetworkManager();

    MessageController* getMessageController() const;

    void run();

private:
    void initNetwork();

    ProcessingUnitHandler* m_processingUnitHandler = nullptr;
    ConnectionTrigger* m_trigger = nullptr;

    uint32_t m_numberOfThreads = 0;
};

} // namespace KyoukoMind

#endif // NETWORK_MANAGER_H
