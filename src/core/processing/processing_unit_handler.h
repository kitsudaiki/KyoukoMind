/**
 *  @file    processing_unit_handler.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef PROCESSING_UNIT_HANDLER_H
#define PROCESSING_UNIT_HANDLER_H

#include <common.h>

namespace KyoukoMind
{
class ProcessingUnit;
class BrickQueue;

class ProcessingUnitHandler
{
public:
    ProcessingUnitHandler();
    ~ProcessingUnitHandler();

    bool initProcessingUnits(const uint16_t numberOfThreads);
    void initNextCycle();
    bool closeAllProcessingUnits();
private:
    std::vector<ProcessingUnit*> m_allProcessingUnits;
};

} // namespace KyoukoMind

#endif // PROCESSING_UNIT_HANDLER_H
