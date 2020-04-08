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
    #endif
}

} // namespace KyoukoMind
