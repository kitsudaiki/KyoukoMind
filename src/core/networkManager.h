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
#include <commonThread.h>

namespace KyoukoMind
{

class Config;
class ClusterHandler;
class ProcessingUnitHandler;
class MessageController;

class NetworkManager : public Kitsune::CommonThread
{

public:
    NetworkManager();

    MessageController* getMessageController() const;
    ClusterHandler* getClusterHandler() const;

    void run();

    static KyoukoMind::Config *m_globalValues;

private:
    void initNetwork();

    ClusterHandler* m_clusterHandler = nullptr;
    ProcessingUnitHandler* m_processingUnitHandler = nullptr;

    uint32_t m_numberOfThreads = 0;
};

}

#endif // MAINLOOP_H
