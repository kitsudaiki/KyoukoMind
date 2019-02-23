/**
 *  @file    clusterTest.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CLUSTERTEST_H
#define CLUSTERTEST_H

#include <common.h>
#include <commonTest.h>

namespace KyoukoMind
{
class Cluster;

class ClusterTest : public Kitsune::CommonTest
{
public:
    ClusterTest();

private:
    void initTestCase();
    void checkInitializing();
    void checkNeighborHandling();
    void checkBlocks();
    void cleanupTestCase();

    Cluster *m_cluster = nullptr;
};

}

#endif // CLUSTERTEST_H
