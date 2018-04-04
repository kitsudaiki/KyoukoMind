#include "clustertest.h"
#include <core/messaging/messagecontroller.h>

#include <core/cluster/nodeCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/emptyCluster.h>

namespace KyoukoMind
{

ClusterTest::ClusterTest() : CommonTest("ClusterTest")
{
    initTestCase();
    checkNodeCluster();
    checkEdgeCluster();
    checkEmptyCluster();
    cleanupTestCase();
}

void ClusterTest::initTestCase()
{
    MessageController contr;
    ClusterID tempId;
    tempId = 0;
    m_nodeCluster = new NodeCluster(tempId, "/tmp", 1000, &contr);
    tempId = 1;
    m_edgeCluster = new EdgeCluster(tempId, "/tmp", &contr);
    tempId = 2;
    m_emptyCluster = new EmptyCluster(tempId, "/tmp", &contr);
}

void ClusterTest::checkNodeCluster()
{
    UNITTEST((int)m_nodeCluster->getNumberOfNodeBlocks(), 4);
    UNITTEST(m_nodeCluster->initNodeBlocks(19), false);

    UNITTEST((int)m_nodeCluster->getNumberOfEdgeBlocks(), 0);

    UNITTEST(m_nodeCluster->initAxonBlocks(2000), true);
    UNITTEST(m_nodeCluster->initAxonBlocks(2000), false);

    UNITTEST((int)m_nodeCluster->getNumberOfEdgeBlocks(), 250);

    KyoChanEdge newEdge;
    newEdge.targetClusterPath = 42;
    newEdge.targetNodeId = 3;
    newEdge.weight = 3.14;
    UNITTEST(m_nodeCluster->addEdge(42, newEdge), true);
    UNITTEST(m_nodeCluster->addEdge(2001, newEdge), false);

    UNITTEST((int)m_nodeCluster->getEdgeBlock()[42].numberOfEdges, 1)
    UNITTEST(m_nodeCluster->getEdgeBlock()[42].edges[0].targetClusterPath, 42);

    for(int i = 0; i < 51; i++) {
        KyoChanEdge newEdge;
        newEdge.targetClusterPath = 42;
        newEdge.targetNodeId = 3;
        newEdge.weight = 3.14;
        m_nodeCluster->addEdge(42, newEdge);
    }

    UNITTEST((int)m_nodeCluster->getNumberOfEdgeBlocks(), 251);
}

void ClusterTest::checkEdgeCluster()
{

}

void ClusterTest::checkEmptyCluster()
{

}

void ClusterTest::cleanupTestCase()
{
    delete m_nodeCluster;
    delete m_edgeCluster;
    delete m_emptyCluster;

    remove("/tmp/cluster_0");
    remove("/tmp/cluster_1");
    remove("/tmp/cluster_2");
}

}
