/**
 *  @file    edgeClusterTest.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef EDGECLUSTERTEST_H
#define EDGECLUSTERTEST_H

#include <common.h>
#include <commonTest.h>

namespace KyoukoMind
{
class EdgeCluster;

class EdgeClusterTest : public Kitsune::CommonTest
{
public:
    EdgeClusterTest();

private:
    void initTestCase();
    void checkInitializing();

    void checkEdgeForwardSections();

    void checkLearningForwardEdgeSection();
    void checkLearningEdgeSection();
    void checkProcessStatusEdge();
    void checkProcessEdgeForwardSection();
    void checkProcessAxon();
    void checkProcessLerningEdge();
    void checkProcessPendingEdge();

    void cleanupTestCase();

    EdgeCluster *m_edgeCluster = nullptr;
};

}

#endif // EDGECLUSTERTEST_H
