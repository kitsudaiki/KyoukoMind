#ifndef STRUCTTEST_H
#define STRUCTTEST_H

#include <common.h>
#include <commonTest.h>

#include <core/structs/kyochanEdges.h>

namespace KyoukoMind
{

class StructTest : public Kitsune::CommonTest
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
