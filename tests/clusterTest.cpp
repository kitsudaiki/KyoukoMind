#include <tests/clusterTest.h>

namespace KyoukoMind
{

ClusterTest::ClusterTest()
{

}

void ClusterTest::initTestCase()
{
    ClusterID tempId;
    m_noteCluster = new NodeCluster(tempId, QString("/tmp"), 1234);
    tempId.x = 1;
    m_edgeCluster = new EdgeCluster(tempId, QString("/tmp"));
    tempId.x = 2;
    m_emptyCluster = new EmptyCluster(tempId, QString("/tmp"));
}

void ClusterTest::checkNodeCluster()
{
    QCOMPARE((int)m_noteCluster->getNumberOfNodeBlocks(), 309);
    QCOMPARE((int)m_noteCluster->getNumberOfEdgeBlocks(), 309);
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
    QFile::remove("/tmp/cluster_0_0_0");
    QFile::remove("/tmp/cluster_1_0_0");
    QFile::remove("/tmp/cluster_2_0_0");
}
 
}
