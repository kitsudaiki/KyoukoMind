/**
 *  @file    main.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <common.h>
#include <src/kyoChanNetwork.h>
#include <tests/runUnitTests.h>

#include <tests/proofOfConcept/demoUserInput.h>

int
main(int argc, char *argv[])
{
    #ifdef RUN_UNIT_TEST
    KyoukoMind::RunUnitTests unitTests;
    unitTests.run();
    #else

    KyoukoMind::KyoukoNetwork* network = new KyoukoMind::KyoukoNetwork();

    KyoukoMind::DemoUserInput input(network->getClusterHandler());
    input.inputLoop();

    #endif
}
