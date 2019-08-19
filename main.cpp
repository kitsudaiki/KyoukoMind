/**
 *  @file    main.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <common.h>
#include <src/kyouko_network.h>
#include <tests/run_unit_tests.h>

int
main(int argc, char *argv[])
{
    #ifdef RUN_UNIT_TEST
    KyoukoMind::RunUnitTests unitTests;
    unitTests.run();
    #else

    KyoukoMind::KyoukoNetwork* network = new KyoukoMind::KyoukoNetwork();
    network->start();
    #endif

    int a = 0;
    std::cin >> a;
    return 0;
}
