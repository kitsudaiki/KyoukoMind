#ifndef STRUCTTEST_H
#define STRUCTTEST_H

#include <common.h>
#include <tests/commonTest.h>

#include <core/structs/kyochanEdges.h>

namespace KyoukoMind
{

class StructTest : public CommonTest
{
public:
    StructTest();

private:
    void initTestCase();
    void testEdgeForwardSection();
    void testEdgeSection();
    void cleanupTestCase();
};

}

#endif // STRUCTTEST_H
