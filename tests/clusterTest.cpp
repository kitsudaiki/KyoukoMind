#include <tests/clusterTest.h>

namespace KyoChan_Network
{

ClusterTest::ClusterTest()
{

}

void ClusterTest::initTestCase()
{
    m_cluster = new NodeCluster(0, 1234, QString("/tmp"));
}

void ClusterTest::checkSizeAfterInit()
{
    QCOMPARE((int)m_cluster->getNumberOfNodeBlocks(), 309);
    QCOMPARE((int)m_cluster->getNumberOfEdgeBlocks(), 309);
}

void ClusterTest::cleanupTestCase()
{
    delete m_cluster;
    QFile::remove("/tmp/cluster0");
}
 
}
