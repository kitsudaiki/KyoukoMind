/**
 *  @file    main.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <common.h>
#include <root_object.h>
#include <tests/run_unit_tests.h>

#include <libKitsunemimiPersistence/logger/logger.h>

int
main(int argc, char *argv[])
{
    Kitsunemimi::Persistence::initConsoleLogger(true);

    KyoukoMind::RootObject* rootObject = new KyoukoMind::RootObject();
    rootObject->start();

    int a = 0;
    std::cin >> a;
    return 0;
}
