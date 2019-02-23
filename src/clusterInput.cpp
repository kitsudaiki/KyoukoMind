/**
 *  @file    clusterInput.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "clusterInput.h"

#include <core/messaging/messages/dataMessage.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>

#include <core/clustering/clusterHandler.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/clustering/globalValuesHandler.h>

/**
 * @brief ClusterInput::ClusterInput
 * @param clusterHandler
 */
ClusterInput::ClusterInput(KyoukoMind::ClusterHandler *clusterHandler)
{
    m_clusterHandler = clusterHandler;

    for(int i = 0; i < NUMBER_OF_NODES_PER_CLUSTER; i++)
    {
        edgeWeights[i] = static_cast<float>((rand() % 50) + 50);
    }
}

/**
 * @brief ClusterInput::init
 * @param id
 * @param connectId
 */
void ClusterInput::init(const uint32_t id, const uint32_t connectId)
{
    m_fakeCluster = new KyoukoMind::NodeCluster(id, 0, 0, "/tmp/test", 2);
    m_clusterHandler->addCluster(id, m_fakeCluster, false);
    m_fakeCluster->setNeighbor(16, connectId);
    m_ougoingBuffer = m_fakeCluster->getOutgoingMessageBuffer(16);

    // connect cluster of the network
    KyoukoMind::Cluster* ingoingCluster = m_clusterHandler->getCluster(connectId);
    ingoingCluster->setNeighbor(0, id);
    m_fakeCluster->setNewConnection(16, ingoingCluster->getIncomingMessageBuffer(0));
}

/**
 * @brief ClusterInput::sendData
 * @param input
 */
void ClusterInput::sendData(const float input)
{
    for(uint16_t i = 0; i < NUMBER_OF_NODES_PER_CLUSTER; i++)
    {
        KyoChanDirectEdgeContainer edge;
        edge.weight = input;
        edge.targetNodeId = i;
        m_ougoingBuffer->addData(&edge);
    }

    m_fakeCluster->finishCycle();
}
