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
#include <libKitsunemimiCommon/test_helper/compare_test_helper.h>
#include <root_object.h>

namespace KyoukoMind
{
class BrickQueue;
class MessageController;
class NetworkInitializer;

class InitTest
        : public Kitsunemimi::CompareTestHelper
{
public:
    InitTest();

private:
    void initTestCase();
    void checkInit();
    void cleanupTestCase();

    NetworkInitializer* m_networkInitializer = nullptr;
    KyoukoMind::RootObject* m_network = nullptr;
    std::string m_testBrickContent = "|0|0|0|0|0|\n"
                                     "|0|2|2|0|0|\n"
                                     "|0|2|1|2|0|\n"
                                     "|0|2|2|0|0|\n"
                                     "|0|0|0|0|0|";
};

} // namespace KyoukoMind

#endif // INIT_TEST_H
