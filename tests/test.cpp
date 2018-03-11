#include "test.h"
#include <tests/clusterTest.h>
#include <tests/messagetest.h>

Test::Test()
{

}

int Test::runClusterTests(int argc, char **argv)
{
    int status = 0;

    KyoukoMind::ClusterTest clusterTest;
    status |= QTest::qExec(&clusterTest, argc, argv);

    KyoukoMind::MessageTest messageTest;
    status |= QTest::qExec(&messageTest, argc, argv);

    return status;
}
