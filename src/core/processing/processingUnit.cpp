/**
 *  @file    processingUnit.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <core/processing/processingUnit.h>
#include <core/cluster/clusterHandler.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingUnit::ProcessingUnit
 * @param clusterHandler
 */
ProcessingUnit::ProcessingUnit(ClusterHandler* clusterHandler)
{
    m_clusterHandler = clusterHandler;
}

/**
 * @brief ProcessingUnit::run
 */
void ProcessingUnit::run()
{
    while(!m_abort)
    {

    }
}

}
