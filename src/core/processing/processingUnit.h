/**
 *  @file    processingUnit.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef PROCESSINGUNIT_H
#define PROCESSINGUNIT_H

#include <common.h>
#include <threading/commonThread.h>

#include <core/messaging/messageObjects/messages.h>
#include <communicationStructs/commonMessages.h>

using Kitsune::Chan::Communication::TransferDataMessage;

struct NeighborInformation;

namespace KyoukoMind
{
class GlobalValuesHandler;
struct Brick;
struct Neighbor;

class ProcessingUnit : public Kitsune::CommonThread
{

public:
    ProcessingUnit();

    void run();

private:
    std::vector<Brick*> m_finishBrickBuffer;

    // cycle
    void processIncomingMessages(Brick *brick);
    bool processIncomingMessage(Brick *brick,
                                const uint8_t side,
                                DataMessage* message);
    void refillWeightMap(Brick* brick,
                         const uint8_t initialSide,
                         Neighbor* neighbors);

    // cycle specific data
    NeighborInformation m_neighborInfo;    
    float m_weightMap[25];
    float m_totalWeightMap = 0.0f;

    bool m_enableMonitoring = true;
    TransferDataMessage* m_monitoringMessage = nullptr;

    bool m_enableClient = true;
    TransferDataMessage* m_clienOutputMessage = nullptr;
};

}

#endif // PROCESSINGUNIT_H
