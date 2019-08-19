/**
 *  @file    run_unit_tests.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "run_unit_tests.h"

#include <tests/core/bricks/brick_methods/buffer_control_methods_test.h>
#include <tests/initializing/init_test.h>
#include <tests/core/messaging/message_buffer_test.h>
#include <tests/core/messaging/message_buffer/incoming_buffer_test.h>

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

} // namespace KyoukoMind
