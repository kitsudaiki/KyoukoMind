/**
 *  @file    global_values_handler.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/bricks/global_values_handler.h>
#include <core/bricks/brick_objects/brick.h>

namespace KyoukoMind
{

GlobalValuesHandler::GlobalValuesHandler()
{

}

/**
 * getter for the global-values
 *
 * @return copy of the global-value-object
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
 * overrides the global values with a new object
 */
void
GlobalValuesHandler::setGlobalValues(const GlobalValues &values)
{
    m_mutex.lock();
    m_globalValues = values;
    m_mutex.unlock();
}

} // namespace KyoukoMind
