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
class ClusterHandler;

class ProcessingUnitHandler
{
public:
    ProcessingUnitHandler(ClusterHandler* clusterHandler);
    ~ProcessingUnitHandler();

    bool initProcessingUnits(const uint16_t numberOfThreads);
    bool closeAllProcessingUnits();
private:
    std::vector<ProcessingUnit*> m_allProcessingUnits;
    ClusterHandler* m_clusterHandler = nullptr;
};

}

#endif // PROCESSINGUNITHANDLER_H
