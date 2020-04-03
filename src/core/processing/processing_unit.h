/**
 *  @file    processing_unit.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef PROCESSING_UNIT_H
#define PROCESSING_UNIT_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>
#include <core/objects/brick.h>

//#include <libKitsunemimiKyoukoCommon/communication_structs/common_messages.h>

//using Kitsunemimi::Chan::Common::TransferDataMessage;

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
    void processIncomingMessages(Brick &brick);
    bool processIncomingMessage(Brick &brick,
                                const uint8_t side,
                                DataBuffer* message);
    void refillWeightMap(Brick &brick,
                         const uint8_t initialSide,
                         Neighbor* neighbors);

    // cycle specific data
    float m_weightMap[25];
    float m_totalWeightMap = 0.0f;

    bool m_enableMonitoring = true;
    //TransferDataMessage* m_monitoringMessage = nullptr;

    bool m_enableClient = true;
    //TransferDataMessage* m_clienOutputMessage = nullptr;
};

} // namespace KyoukoMind

#endif // PROCESSING_UNIT_H
