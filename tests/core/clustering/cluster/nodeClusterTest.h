/**
 *  @file    nodeClusterTest.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef NODECLUSTERTEST_H
#define NODECLUSTERTEST_H

#include <common.h>
#include <commonTest.h>

namespace KyoukoMind
{
class NodeCluster;

class NodeClusterTest : public Kitsune::CommonTest
{
public:
    NodeClusterTest();

private:
    void initTestCase();
    void checkInitializing();
    void cleanupTestCase();

    NodeCluster *m_nodeCluster = nullptr;
};

}

#endif // NODECLUSTERTEST_H
