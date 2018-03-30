#include "clustertest.h"
#include <core/messaging/messagecontroller.h>

#include <core/cluster/nodeCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/emptyCluster.h>

namespace KyoukoMind
{

ClusterTest::ClusterTest()
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
    m_noteCluster = new NodeCluster(tempId, "/tmp", 1000, &contr);
    tempId = 1;
    m_edgeCluster = new EdgeCluster(tempId, "/tmp", &contr);
    tempId = 2;
    m_emptyCluster = new EmptyCluster(tempId, "/tmp", &contr);
}

void ClusterTest::checkNodeCluster()
{
    UNITTEST((int)m_noteCluster->getNumberOfNodeBlocks(), 7);
    UNITTEST((int)m_noteCluster->getNumberOfEdgeBlocks(), 1000);
}

void ClusterTest::checkEdgeCluster()
{

}

void ClusterTest::checkEmptyCluster()
{

}

void ClusterTest::cleanupTestCase()
{
    delete m_noteCluster;
    delete m_edgeCluster;
    delete m_emptyCluster;
    //QFile::remove("/tmp/cluster_0_0_0");
    //QFile::remove("/tmp/cluster_1_0_0");
    //QFile::remove("/tmp/cluster_2_0_0");
}

}
