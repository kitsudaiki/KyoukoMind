/**
 *  @file    globalValuesHandler.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/bricks/globalValuesHandler.h>
#include <core/bricks/brickObjects/brick.h>

namespace KyoukoMind
{

GlobalValuesHandler::GlobalValuesHandler()
{

}

/**
 * @brief GlobalValuesHandler::getGlobalValues
 * @return
 */
GlobalValues
GlobalValuesHandler::getGlobalValues()
{
    GlobalValues values;
    m_mutex.lock();
    values = m_globalValues;
    m_mutex.unlock();
    return values;
}

/**
 * @brief GlobalValuesHandler::setGlobalValues
 * @param values
 */
void
GlobalValuesHandler::setGlobalValues(const GlobalValues &values)
{
    m_mutex.lock();
    m_globalValues = values;
    m_mutex.unlock();
}

}
