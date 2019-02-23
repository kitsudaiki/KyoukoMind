/**
 *  @file    edgeClusterTest.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "edgeClusterTest.h"

#include <core/clustering/cluster/edgeCluster.h>
#include <core/structs/kyochanEdges.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{

EdgeClusterTest::EdgeClusterTest() : Kitsune::CommonTest("EdgeClusterTest")
{
    initTestCase();
    checkInitializing();

    checkEdgeForwardSections();

    checkLearningForwardEdgeSection();
    checkLearningEdgeSection();
    checkProcessStatusEdge();
    checkProcessEdgeForwardSection();
    checkProcessAxon();
    checkProcessLerningEdge();
    checkProcessPendingEdge();

    cleanupTestCase();
}

void EdgeClusterTest::initTestCase()
{
    m_edgeCluster = new EdgeCluster(1337, 0,0,"/tmp/");
}


void EdgeClusterTest::checkInitializing()
{
    UNITTEST(m_edgeCluster->initForwardEdgeSectionBlocks(10), true)
    UNITTEST(m_edgeCluster->initForwardEdgeSectionBlocks(10), false)

    UNITTEST(m_edgeCluster->getNumberOfForwardEdgeSections(), 10)
    UNITTEST(m_edgeCluster->getNumberOfForwardEdgeSectionBlocks(), 1)

    UNITTEST(m_edgeCluster->addEmptyForwardEdgeSection(1, 2), 10)
    UNITTEST(m_edgeCluster->getNumberOfForwardEdgeSections(), 11)

    #ifdef RUN_UNIT_TEST
    KyoChanForwardEdgeSection edge = m_edgeCluster->getForwardEdgeSectionBlock_public()[10];
    UNITTEST(edge.sourceId, 2)
    UNITTEST(edge.sourceSide, 1)
    #endif
}

void EdgeClusterTest::checkEdgeForwardSections()
{

}

void EdgeClusterTest::checkLearningForwardEdgeSection()
{

}

void EdgeClusterTest::checkLearningEdgeSection()
{

}

void EdgeClusterTest::checkProcessStatusEdge()
{

}

void EdgeClusterTest::checkProcessEdgeForwardSection()
{

}

void EdgeClusterTest::checkProcessAxon()
{

}

void EdgeClusterTest::checkProcessLerningEdge()
{

}

void EdgeClusterTest::checkProcessPendingEdge()
{

}

void EdgeClusterTest::cleanupTestCase()
{
    delete m_edgeCluster;
}

}
