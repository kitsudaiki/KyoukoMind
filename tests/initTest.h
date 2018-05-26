/**
 *  @file    initTest.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef INITTEST_H
#define INITTEST_H

#include <common.h>
#include <tests/commonTest.h>

namespace KyoukoMind
{
class ClusterHandler;
class EdgeCluster;
class MessageController;
class NetworkInitializer;

class InitTest : public CommonTest
{
public:
    InitTest();

private:
    void initTestCase();
    void checkInit();
    void cleanupTestCase();

    ClusterHandler* m_testClusterHandler = nullptr;
    MessageController* m_testMessageController = nullptr;
    NetworkInitializer* m_networkInitializer = nullptr;

    std::string m_testClusterContent = "|0|0|0|0|0|\n|0|2|2|0|0|\n|0|2|1|2|0|\n|0|2|2|0|0|\n|0|0|0|0|0|";
};

}

#endif // INITTEST_H
