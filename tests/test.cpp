#include "test.h"
#include <tests/clusterTest.h>

Test::Test()
{

}

int Test::runClusterTests(int argc, char **argv)
{
    int status = 0;

    KyoukoMind::ClusterTest clusterTest;
    status |= QTest::qExec(&clusterTest, argc, argv);

    return status;
}
