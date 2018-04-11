/**
 *  @file    processingUnitHandler.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef PROCESSINGUNITHANDLER_H
#define PROCESSINGUNITHANDLER_H

#include <common.h>

namespace KyoukoMind
{
class ProcessingUnit;

class ProcessingUnitHandler
{
public:
    ProcessingUnitHandler();
    ~ProcessingUnitHandler();

    bool initProcessingUnits(const uint16_t numberOfThreads);
    bool closeAllProcessingUnits();
private:
    std::vector<ProcessingUnit*> m_allProcessingUnits;
};

}

#endif // PROCESSINGUNITHANDLER_H
