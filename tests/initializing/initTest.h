/**
 *  @file    initTest.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef INITTEST_H
#define INITTEST_H

#include <common.h>
#include <testing/commonTest.h>
#include <kyoukoNetwork.h>

namespace KyoukoMind
{
class BrickHandler;
class MessageController;
class NetworkInitializer;

class InitTest : public Kitsune::CommonTest
{
public:
    InitTest();

private:
    void initTestCase();
    void checkInit();
    void cleanupTestCase();

    NetworkInitializer* m_networkInitializer = nullptr;
    KyoukoMind::KyoukoNetwork* m_network = nullptr;
    std::string m_testBrickContent = "|0|0|0|0|0|\n|0|2|2|0|0|\n|0|2|1|2|0|\n|0|2|2|0|0|\n|0|0|0|0|0|";
};

}

#endif // INITTEST_H
