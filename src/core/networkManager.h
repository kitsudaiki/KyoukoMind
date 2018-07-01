/**
 *  @file    networkManager.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef NETTHREADMANAGER_H
#define NETTHREADMANAGER_H

#include <common.h>
#include <core/processing/processingThreads/commonThread.h>
#include <common/typedefs.h>

namespace KyoukoMind
{

class Config;
class ClusterHandler;
class ProcessingUnitHandler;
class MessageController;

class NetworkManager : public CommonThread
{

public:
    NetworkManager();

    MessageController* getMessageController() const;
    ClusterHandler* getClusterHandler() const;

    void run();

private:
    void initNetwork();

    ClusterHandler* m_clusterHandler = nullptr;
    ProcessingUnitHandler* m_processingUnitHandler = nullptr;

    uint32_t m_numberOfThreads = 0;
};

}

#endif // MAINLOOP_H
