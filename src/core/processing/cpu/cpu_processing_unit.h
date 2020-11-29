/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef PROCESSING_UNIT_H
#define PROCESSING_UNIT_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

namespace Kitsunemimi {
class Barrier;
}

class GlobalValuesHandler;
class Segment;
class Brick;

class CpuProcessingUnit
        : public Kitsunemimi::Thread
{

public:
    CpuProcessingUnit();

    void run();

    Kitsunemimi::Barrier* m_phase1 = nullptr;
    Kitsunemimi::Barrier* m_phase2 = nullptr;
    Kitsunemimi::Barrier* m_phase3 = nullptr;

private:
    DataBuffer m_clientBuffer;
    DataBuffer m_monitoringBuffer;
};

#endif // PROCESSING_UNIT_H
