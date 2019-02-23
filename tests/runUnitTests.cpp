/**
 *  @file    runUnitTests.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "runUnitTests.h"

namespace KyoukoMind
{

RunUnitTests::RunUnitTests()
{

}

void
RunUnitTests::run()
{
    #ifdef RUN_UNIT_TEST
    KyoukoMind::InitTest();
    KyoukoMind::ClusterTest();
    KyoukoMind::EdgeClusterTest();
    #endif

    //KyoukoMind::MessageTest();
}

}
