/**
 *  @file    processing_unit_handler.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/processing/processing_unit_handler.h>
#include <core/processing/processing_unit.h>
#include <core/brick_handler.h>

#include <libKitsunemimiCommon/threading/thread.h>

#include <root_object.h>

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
        ProcessingUnit* newUnit = new ProcessingUnit();
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
        ProcessingUnit* unit = m_allProcessingUnits.at(i);
        unit->stopThread();
        delete unit;
    }

    m_allProcessingUnits.clear();;
    return true;
}

} // namespace KyoukoMind
