/**
 *  @file    processingUnitHandler.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef PROCESSINGUNITHANDLER_H
#define PROCESSINGUNITHANDLER_H

#include <common.h>

namespace KyoukoMind
{
class ProcessingUnit;
class ClusterHandler;

class ProcessingUnitHandler
{
public:
    ProcessingUnitHandler(ClusterHandler* clusterHandler);
    ~ProcessingUnitHandler();

    bool initProcessingUnits(const uint16_t numberOfThreads);
    void initNextCycle();
    bool closeAllProcessingUnits();
private:
    std::vector<ProcessingUnit*> m_allProcessingUnits;
    ClusterHandler* m_clusterHandler = nullptr;
};

}

#endif // PROCESSINGUNITHANDLER_H
