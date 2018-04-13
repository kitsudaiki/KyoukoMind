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

#include <common/typedefs.h>

namespace KyoukoMind
{

class Config;
class ClusterHandler;
class ProcessingUnitHandler;
class MessageController;

class NetworkManager
{

public:
    NetworkManager();

    MessageController* getMessageController() const;

private:
    ClusterHandler* m_clusterHandler = nullptr;
    ProcessingUnitHandler* m_processingUnitHandler = nullptr;
    MessageController* m_messageController = nullptr;

    uint32_t m_numberOfThreads = 0;
};

}

#endif // MAINLOOP_H
