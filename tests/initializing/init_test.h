/**
 *  @file    init_test.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef INIT_TEST_H
#define INIT_TEST_H

#include <common.h>
#include <libKitsunemimiCommon/test.h>
#include <kyouko_network.h>

namespace KyoukoMind
{
class BrickHandler;
class MessageController;
class NetworkInitializer;

class InitTest
        : public Kitsunemimi::Common::Test
{
public:
    InitTest();

private:
    void initTestCase();
    void checkInit();
    void cleanupTestCase();

    NetworkInitializer* m_networkInitializer = nullptr;
    KyoukoMind::KyoukoNetwork* m_network = nullptr;
    std::string m_testBrickContent = "|0|0|0|0|0|\n"
                                     "|0|2|2|0|0|\n"
                                     "|0|2|1|2|0|\n"
                                     "|0|2|2|0|0|\n"
                                     "|0|0|0|0|0|";
};

} // namespace KyoukoMind

#endif // INIT_TEST_H
