/**
 *  @file    processingTest.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "processingTest.h"
#include <messageQueues/incomingMessageBuffer.h>
#include <messageQueues/outgoingMessageBuffer.h>

#include <core/clustering/cluster/nodeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>

#include <core/processing/processingThreads/cpu/edgeClusterProcessing.h>

namespace KyoukoMind
{

/**
 * @brief ProcessingTest::ProcessingTest
 */
ProcessingTest::ProcessingTest() : CommonTest("ProcessingTest")
{
    initTestCase();
    checkProcessing();
    cleanupTestCase();
}

/**
 * @brief ProcessingTest::initTestCase
 */
void ProcessingTest::initTestCase()
{
    //m_nodeProcessing = new ClusterProcessing(m_nextChooser);

    NodeCluster* initCluster = new NodeCluster(1337, "/tmp/test", 42);

    initCluster->setNeighbor(15, 1);

    m_nodeCluster1 = new NodeCluster(1, "/tmp/test", 1);
    m_nodeCluster2 = new NodeCluster(2, "/tmp/test", 1);
    m_nodeCluster3 = new NodeCluster(3, "/tmp/test", 1);

    m_nodeCluster1->setNeighbor(3, 2);

    m_nodeCluster1->getNodeBlock()[0].targetClusterPath = 3;
    m_nodeCluster1->getNodeBlock()[0].targetAxonId = 0;

    m_nodeCluster2->setNeighbor(3, 3);
    m_nodeCluster3->setNeighbor(3, 2);
}

/**
 * @brief ProcessingTest::checkProcessing
 */
void ProcessingTest::checkProcessing()
{
}

/**
 * @brief ProcessingTest::cleanupTestCase
 */
void ProcessingTest::cleanupTestCase()
{
    delete m_nodeCluster1;
    delete m_nodeCluster2;
    delete m_nodeCluster3;
}

}
