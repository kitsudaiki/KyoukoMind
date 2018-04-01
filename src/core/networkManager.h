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
    uint32_t* getMindDimension();

private:
    ClusterHandler* m_clusterManager = nullptr;
    ProcessingUnitHandler* m_processingUnitHandler = nullptr;
    MessageController* m_messageController = nullptr;

    uint32_t m_numberOfThreads = 0;

    uint32_t m_mindDimensions[3];
};
}

#endif // MAINLOOP_H
