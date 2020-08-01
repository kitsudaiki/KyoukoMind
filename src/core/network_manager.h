/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

namespace Kitsunemimi {
class Barrier;
}

namespace KyoukoMind
{

class ProcessingUnitHandler;

class NetworkManager
        : public Kitsunemimi::Thread
{

public:
    NetworkManager();

    void run();

private:
    bool initNetwork();

    ProcessingUnitHandler* m_processingUnitHandler = nullptr;
    Kitsunemimi::Barrier* m_phase1 = nullptr;
    Kitsunemimi::Barrier* m_phase2 = nullptr;
    Kitsunemimi::Barrier* m_phase3 = nullptr;

    uint32_t m_numberOfThreads = 0;
};

} // namespace KyoukoMind

#endif // NETWORK_MANAGER_H
