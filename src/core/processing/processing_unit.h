/**
 *  @file    processing_unit.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef PROCESSING_UNIT_H
#define PROCESSING_UNIT_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>
#include <core/network_segment.h>

namespace KyoukoMind
{
class GlobalValuesHandler;

class ProcessingUnit
        : public Kitsunemimi::Thread
{

public:
    ProcessingUnit();

    void run();

    std::vector<Brick*> m_finishBrickBuffer;

private:

    DataBuffer m_clientBuffer;
    DataBuffer m_monitoringBuffer;
};

} // namespace KyoukoMind

#endif // PROCESSING_UNIT_H
