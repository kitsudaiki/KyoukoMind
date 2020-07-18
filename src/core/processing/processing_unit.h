/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef PROCESSING_UNIT_H
#define PROCESSING_UNIT_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

namespace KyoukoMind
{
class GlobalValuesHandler;
class Segment;
class Brick;

class ProcessingUnit
        : public Kitsunemimi::Thread
{

public:
    ProcessingUnit();

    void run();

private:
    DataBuffer m_clientBuffer;
    DataBuffer m_monitoringBuffer;
};

} // namespace KyoukoMind

#endif // PROCESSING_UNIT_H
