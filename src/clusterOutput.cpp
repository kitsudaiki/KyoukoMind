/**
 *  @file    clusterOutput.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "clusterOutput.h"

#include <core/messaging/messages/dataMessage.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>

#include <core/clustering/clusterHandler.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/clustering/globalValuesHandler.h>

/**
 * @brief ClusterOutput::ClusterOutput
 * @param clusterHandler
 */
ClusterOutput::ClusterOutput(KyoukoMind::ClusterHandler* clusterHandler)
{
    m_clusterHandler = clusterHandler;
}

/**
 * @brief ClusterOutput::init
 * @param id
 * @param connectId
 */
void ClusterOutput::init(const uint32_t id, const uint32_t connectId)
{
    m_outgoingCluster = static_cast<KyoukoMind::NodeCluster*>(m_clusterHandler->getCluster(connectId));
    m_outgoingCluster->setAsOutput();
}

/**
 * @brief ClusterOutput::getOutput
 * @return
 */
std::string ClusterOutput::getOutput()
{
    m_outBuffer[m_outBufferPos] = m_outgoingCluster->getSummedValue();
    m_outBufferPos = (m_outBufferPos + 1) % 10;

    float result = 0.0f;
    for(uint32_t i = 0; i < 10; i++)
    {
        result += m_outBuffer[i];
    }
    result /= 10.0f;

    if(result >= 0.1f)
    {
        std::ostringstream ss;
        ss << result;
        std::string s(ss.str());
        return ss.str();
    }
    return "0";
}

/**
 * @brief ClusterOutput::setLearningOverride
 * @param value
 */
void ClusterOutput::setLearningOverride(float value)
{
    m_outgoingCluster->setLearningOverride(value);
}
