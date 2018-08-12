/**
 *  @file    processingUnitHandler.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/processing/processingUnitHandler.h>
#include <core/processing/processingUnit.h>
#include <core/processing/cpu/cpuProcessingUnit.h>
#include <core/clustering/clusterHandler.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingUnitHandler::ProcessingUnitHandler
 * @param clusterHandler
 */
ProcessingUnitHandler::ProcessingUnitHandler(ClusterHandler* clusterHandler)
{
    m_clusterHandler = clusterHandler;
}

/**
 * @brief ProcessingUnitHandler::~ProcessingUnitHandler
 */
ProcessingUnitHandler::~ProcessingUnitHandler()
{
    closeAllProcessingUnits();
}

/**
 * @brief ProcessingUnitHandler::initProcessingUnits
 * @param numberOfThreads
 * @return
 */
bool ProcessingUnitHandler::initProcessingUnits(const uint16_t numberOfThreads)
{
    if(m_allProcessingUnits.size() != 0) {
        return false;
    }
    for(uint16_t i = 0; i < numberOfThreads; i++)
    {
        ProcessingUnit* newUnit = new CpuProcessingUnit(m_clusterHandler->getClusterQueue());
        m_allProcessingUnits.push_back(newUnit);
        newUnit->start();
    }
    return true;
}

/**
 * @brief ProcessingUnitHandler::initNextCycle
 */
void ProcessingUnitHandler::initNextCycle()
{
    for(uint32_t i = 0; i < m_allProcessingUnits.size(); i++) {
        m_allProcessingUnits.at(i)->continueThread();
    }
}

/**
 * @brief ProcessingUnitHandler::closeAllProcessingUnits close all processing-units, if exist
 * @return false, if no processing-units exist, else true
 */
bool ProcessingUnitHandler::closeAllProcessingUnits()
{
    if(m_allProcessingUnits.size() == 0) {
        return false;
    }
    for(uint32_t i = 0; i < m_allProcessingUnits.size(); i++)
    {
        CpuProcessingUnit* unit = static_cast<CpuProcessingUnit*>(m_allProcessingUnits.at(i));
        unit->stop();
        delete unit;
    }
    m_allProcessingUnits.clear();;
    return true;
}

}
