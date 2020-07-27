/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <core/processing/processing_unit_handler.h>
#include <core/processing/cpu/cpu_processing_unit.h>

#include <libKitsunemimiCommon/threading/thread.h>

#include <kyouko_root.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingUnitHandler::ProcessingUnitHandler
 * @param brickHandler
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
bool
ProcessingUnitHandler::initProcessingUnits(const uint16_t numberOfThreads)
{
    if(m_allProcessingUnits.size() != 0) {
        return false;
    }

    for(uint16_t i = 0; i < numberOfThreads; i++)
    {
        CpuProcessingUnit* newUnit = new CpuProcessingUnit();
        m_allProcessingUnits.push_back(newUnit);
        newUnit->startThread();
    }

    return true;
}

/**
 * @brief ProcessingUnitHandler::initNextCycle
 */
void
ProcessingUnitHandler::initNextCycle()
{
    for(uint32_t i = 0; i < m_allProcessingUnits.size(); i++)
    {
        m_allProcessingUnits.at(i)->continueThread();
    }
}

/**
 * @brief ProcessingUnitHandler::closeAllProcessingUnits close all processing-units, if exist
 * @return false, if no processing-units exist, else true
 */
bool
ProcessingUnitHandler::closeAllProcessingUnits()
{
    if(m_allProcessingUnits.size() == 0) {
        return false;
    }

    for(uint32_t i = 0; i < m_allProcessingUnits.size(); i++)
    {
        CpuProcessingUnit* unit = m_allProcessingUnits.at(i);
        unit->stopThread();
        delete unit;
    }

    m_allProcessingUnits.clear();

    return true;
}

} // namespace KyoukoMind
