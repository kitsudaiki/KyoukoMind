#include <core/networkManager.h>
#include <core/cluster/clusterHandler.h>
#include <core/cluster/cluster.h>
#include <settings/initialFileInput.h>
#include <settings/config.h>
#include <KyoChanNetwork.h>

namespace KyoukoMind
{

/**
 * @brief NetManager::NetManager
 */
NetworkManager::NetworkManager()
{
    bool ok = false;
    initNodeNetwork();
}

/**
 * @brief NetManager::initNodeNetwork
 */
void NetworkManager::initNodeNetwork()
{
    InitialFileInput initial;
}

/**
 * @brief NetManager::startLoops
 */
void NetworkManager::startLoops()
{
    for(unsigned int i = 0; i < m_numberOfThreads; i++)
    {
        //NetThread* netThread = new NetThread(i);
        //m_netThreads.push_back(netThread);
        //netThread->startThread();
    }

    /*m_loopEnd = chronoClock::now();
    long timeSpan = std::chrono::duration_cast<chronoMicroSec>(m_loopEnd-m_loopStart).count();
    long timeDiff = (1000000 / LOOP_RUNS_PER_SECOND) - timeSpan;
    if(timeDiff > 0)
    {
        std::this_thread::sleep_for(chronoMicroSec(timeDiff));
    }*/
}

/**
 * @brief NetManager::setThread
 * @param thread
 * @return
 */
bool NetworkManager::setThread(QThread *thread)
{
    if(m_thread == nullptr)
    {
        m_thread = thread;
        return true;
    }
    return false;
}

}
