/**
 *  @file    runUnitTests.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef RUNUNITTESTS_H
#define RUNUNITTESTS_H

#include <common.h>

#include <tests/core/clustering/cluster/clusterTest.h>
#include <tests/core/clustering/cluster/edgeClusterTest.h>
#include <tests/core/messaging/message/messageTest.h>
#include <tests/initializing/initTest.h>

namespace KyoukoMind
{

class RunUnitTests
{
public:
    RunUnitTests();

    void run();
};

}

#endif // RUNUNITTESTS_H
