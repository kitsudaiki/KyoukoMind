/**
 *  @file    clusterTest.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CLUSTERTEST_H
#define CLUSTERTEST_H

#include <common.h>
#include <commonTest.h>

namespace KyoukoMind
{

class NodeCluster;
class EdgeCluster;

class ClusterTest : public Kitsune::CommonTest
{

public:
    ClusterTest();

private:
    void initTestCase();
    void checkInitializing();
    void checkMessageBuffer();
    void checkNodeCluster();
    void checkEdgeCluster();
    void checkEmptyCluster();
    void cleanupTestCase();

    NodeCluster *m_nodeCluster = nullptr;
    EdgeCluster *m_edgeCluster = nullptr;
    NodeCluster* m_messageBufferCluster = nullptr;
};

}

#endif // CLUSTERTEST_H
