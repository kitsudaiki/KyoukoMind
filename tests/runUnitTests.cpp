/**
 *  @file    runUnitTests.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "runUnitTests.h"

#include <tests/core/bricks/brickMethods/bufferControlMethodsTest.h>
#include <tests/initializing/initTest.h>
#include <tests/core/messaging/messageBufferTest.h>
#include <tests/core/messaging/messageMarker/incomingBufferTest.h>

namespace KyoukoMind
{

RunUnitTests::RunUnitTests()
{

}

void
RunUnitTests::run()
{
    #ifdef RUN_UNIT_TEST
    KyoukoMind::BufferControlMethodsTest();
    KyoukoMind::InitTest();
    KyoukoMind::MessageBufferTest();
    KyoukoMind::IncomingBufferTest();
    #endif
}

}
