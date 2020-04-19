/**
 *  @file    main_tests.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <tests/core/bricks/brick_methods/buffer_control_methods_test.h>
#include <tests/initializing/init_test.h>

int
main(int argc, char *argv[])
{
    KyoukoMind::BufferControlMethodsTest();
    KyoukoMind::InitTest();
    return 0;
}
