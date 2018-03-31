#ifndef CLUSTERTEST_H
#define CLUSTERTEST_H

#include <common.h>

namespace KyoukoMind
{

class NodeCluster;
class EdgeCluster;
class EmptyCluster;

class ClusterTest
{

public:
    ClusterTest();

private:
    void initTestCase();
    void checkNodeCluster();
    void checkEdgeCluster();
    void checkEmptyCluster();
    void cleanupTestCase();

    NodeCluster *m_nodeCluster = nullptr;
    EdgeCluster *m_edgeCluster = nullptr;
    EmptyCluster *m_emptyCluster = nullptr;
};

}

#endif // CLUSTERTEST_H
