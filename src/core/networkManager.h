/**
 *  @file    networkManager.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef NETTHREADMANAGER_H
#define NETTHREADMANAGER_H

#include <common.h>
#include <threading/commonThread.h>

namespace KyoukoMind
{

class Config;
class BrickHandler;
class ProcessingUnitHandler;
class MessageController;
class ConnectionTrigger;

class NetworkManager : public Kitsune::CommonThread
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

}

#endif // MAINLOOP_H
