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
#include <core/objects/network_segment.h>

namespace KyoukoMind
{
class GlobalValuesHandler;

class ProcessingUnit
        : public Kitsunemimi::Thread
{

public:
    ProcessingUnit();

    void run();

private:
    std::vector<Brick*> m_finishBrickBuffer;

    // cycle
    void processIncomingMessages(NetworkSegment &segment,
                                 Brick &brick);
    bool processIncomingMessage(NetworkSegment &segment,
                                Brick &brick,
                                const uint8_t side,
                                DataBuffer* message);

    DataBuffer m_clientBuffer;
    DataBuffer m_monitoringBuffer;
};

} // namespace KyoukoMind

#endif // PROCESSING_UNIT_H
