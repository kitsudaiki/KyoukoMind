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
#include <core/processing/cpuProcessingUnit.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingUnitHandler::ProcessingUnitHandler
 */
ProcessingUnitHandler::ProcessingUnitHandler()
{

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
        ProcessingUnit* newUnit = new CpuProcessingUnit();
        m_allProcessingUnits.push_back(newUnit);
        newUnit->start();
    }
    return true;
}

/**
 * @brief ProcessingUnitHandler::closeAllProcessingUnits
 * @return
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
