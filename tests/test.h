#ifndef TEST_H
#define TEST_H

#include <QTest>

class Test
{
public:
    Test();
    int runClusterTests(int argc, char **argv);
};

#endif // TEST_H
