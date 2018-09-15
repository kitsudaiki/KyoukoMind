/**
 *  @file    clusterTest.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "clusterTest.h"

#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>

#include <messageQueues/incomingMessageBuffer.h>
#include <messageQueues/outgoingMessageBuffer.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{

ClusterTest::ClusterTest() : Kitsune::CommonTest("ClusterTest")
{
    initTestCase();
    checkInitializing();
    checkMessageBuffer();
    checkEdgeCluster();
    checkNodeCluster();
    checkEmptyCluster();
    cleanupTestCase();
}

void ClusterTest::initTestCase()
{
    ClusterID tempId;
    tempId = 0;
    m_nodeCluster = new NodeCluster(tempId, "/tmp", 1000);
    tempId = 1;
    m_edgeCluster = new EdgeCluster(tempId, "/tmp");

    m_messageBufferCluster = new NodeCluster(1337, "/tmp/test");
}


/**
 * @brief ClusterTest::checkInitializing
 */
void ClusterTest::checkInitializing()
{
    UNITTEST(m_messageBufferCluster->setNeighbor(0, 1337), true);
    UNITTEST(m_messageBufferCluster->setNeighbor(16, 1337), true);
    UNITTEST(m_messageBufferCluster->setNeighbor(17, 1337), false);


    UNITTEST(m_messageBufferCluster->setNewConnection(0, m_messageBufferCluster->getIncomingMessageBuffer(16)), true);
    UNITTEST(m_messageBufferCluster->setNewConnection(16, m_messageBufferCluster->getIncomingMessageBuffer(0)), true);
    UNITTEST(m_messageBufferCluster->setNewConnection(17, m_messageBufferCluster->getIncomingMessageBuffer(0)), false);
}

/**
 * @brief ClusterTest::checkMessageBuffer
 */
void ClusterTest::checkMessageBuffer()
{
    KyoChanForwardEdgeContainer edge;

    // get
    UNITTEST(m_messageBufferCluster->isBufferReady(), true);
    UNITTEST(m_messageBufferCluster->getIncomingMessageBuffer(0)->getMessage()->getPayloadSize(), 0)
    UNITTEST(m_messageBufferCluster->getIncomingMessageBuffer(16)->getMessage()->getPayloadSize(), 0)

    m_messageBufferCluster->finishCycle(0);
    UNITTEST(m_messageBufferCluster->isBufferReady(), true);

    // get
    m_messageBufferCluster->getIncomingMessageBuffer(0)->getMessage();
    m_messageBufferCluster->getIncomingMessageBuffer(16)->getMessage();

    edge.targetEdgeSectionId = 1;
    // set
    UNITTEST(m_messageBufferCluster->getOutgoingMessageBuffer(0)->addData(&edge), true);
    UNITTEST(m_messageBufferCluster->getOutgoingMessageBuffer(16)->addData(&edge), true);
    UNITTEST(m_messageBufferCluster->getOutgoingMessageBuffer(17)->addData(&edge), false);

    m_messageBufferCluster->finishCycle(0);
    UNITTEST(m_messageBufferCluster->isBufferReady(), true);

    // get
    Kitsune::MindMessaging::DataMessage* message0 = m_messageBufferCluster->getIncomingMessageBuffer(0)->getMessage();
    Kitsune::MindMessaging::DataMessage* message16 = m_messageBufferCluster->getIncomingMessageBuffer(16)->getMessage();
    UNITTEST(message0->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))
    UNITTEST(message16->getPayloadSize(), sizeof(KyoChanForwardEdgeContainer))

    m_messageBufferCluster->finishCycle(0);
    UNITTEST(m_messageBufferCluster->isBufferReady(), true);

    KyoChanAxonEdgeContainer edge2;
    edge2.targetAxonId = 1;

    // get
    m_messageBufferCluster->getIncomingMessageBuffer(0)->getMessage();
    m_messageBufferCluster->getIncomingMessageBuffer(16)->getMessage();

    // set
    UNITTEST(m_messageBufferCluster->getOutgoingMessageBuffer(0)->addData(&edge2), true);
    UNITTEST(m_messageBufferCluster->getOutgoingMessageBuffer(16)->addData(&edge2), true);
    UNITTEST(m_messageBufferCluster->getOutgoingMessageBuffer(17)->addData(&edge2), false);

    m_messageBufferCluster->finishCycle(0);
    UNITTEST(m_messageBufferCluster->isBufferReady(), true);

    // get
    UNITTEST(m_messageBufferCluster->getIncomingMessageBuffer(0)->getMessage()->getPayloadSize(), sizeof(KyoChanAxonEdgeContainer))
    UNITTEST(m_messageBufferCluster->getIncomingMessageBuffer(16)->getMessage()->getPayloadSize(), sizeof(KyoChanAxonEdgeContainer))


    m_messageBufferCluster->finishCycle(0);
    UNITTEST(m_messageBufferCluster->isBufferReady(), true);

    UNITTEST(m_messageBufferCluster->getIncomingMessageBuffer(0)->getMessage()->getPayloadSize(), 0)
    UNITTEST(m_messageBufferCluster->getIncomingMessageBuffer(16)->getMessage()->getPayloadSize(), 0)

}

void ClusterTest::checkEdgeCluster()
{
    UNITTEST(m_edgeCluster->initForwardEdgeSectionBlocks(2000), true);
    UNITTEST(m_edgeCluster->addEmptyForwardEdgeSection(42, 42), 2000);
    UNITTEST(m_edgeCluster->addEmptyForwardEdgeSection(42, 42), 2001);
}

void ClusterTest::checkNodeCluster()
{
    UNITTEST((int)m_nodeCluster->getNumberOfNodeBlocks(), 6);
    UNITTEST(m_nodeCluster->initNodeBlocks(19), false);

    KyoChanEdge newEdge;
    newEdge.targetNodeId = 3;
    newEdge.weight = 3.14f;

    for(int i = 0; i < 51; i++) {
        KyoChanEdge newEdge;
        newEdge.targetNodeId = 3;
        newEdge.weight = 3.14f;
        m_nodeCluster->addEdge(42, newEdge);
    }

    //UNITTEST((int)m_nodeCluster->getNumberOfEdgeBlocks(), 251);
}

void ClusterTest::checkEmptyCluster()
{

}

void ClusterTest::cleanupTestCase()
{
    delete m_nodeCluster;
    delete m_edgeCluster;

    remove("/tmp/cluster_0");
    remove("/tmp/cluster_1");
    remove("/tmp/cluster_2");
}

}
