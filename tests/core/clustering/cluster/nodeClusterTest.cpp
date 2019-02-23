/**
 *  @file    nodeClusterTest.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "nodeClusterTest.h"

#include <core/clustering/cluster/nodeCluster.h>

#include <core/messaging/incomingMessageBuffer.h>
#include <core/messaging/outgoingMessageBuffer.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{

NodeClusterTest::NodeClusterTest() : Kitsune::CommonTest("NodeClusterTest")
{
    initTestCase();
    checkInitializing();
    cleanupTestCase();
}

void NodeClusterTest::initTestCase()
{

}


void NodeClusterTest::checkInitializing()
{

}


void NodeClusterTest::cleanupTestCase()
{
    delete m_nodeCluster;
}

}
