/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <common.h>
#include <random>
#include <libKitsunemimiCommon/threading/thread.h>

namespace Kitsunemimi {
class Barrier;
}

class ProcessingUnitHandler;

class NetworkManager
        : public Kitsunemimi::Thread
{

public:
    NetworkManager();

    void run();

private:
    bool initNetwork();
    void calcNewLearningValue();

    ProcessingUnitHandler* m_processingUnitHandler = nullptr;
    Kitsunemimi::Barrier* m_phase1 = nullptr;
    Kitsunemimi::Barrier* m_phase2 = nullptr;
    Kitsunemimi::Barrier* m_phase3 = nullptr;

    uint32_t m_numberOfThreads = 0;
    std::string m_testMonitoring = "";

    float m_oldIndex = 0.0f;
    std::vector<float> m_actualOutput;
    std::vector<float> m_should;
};

#endif // NETWORK_MANAGER_H
