#include "structTest.h"

namespace KyoukoMind
{

/**
 * @brief StructTest::StructTest
 */
StructTest::StructTest(): CommonTest("StructTest")
{
    initTestCase();
    testEdgeForwardSection();
    testEdgeSection();
    cleanupTestCase();
}

/**
 * @brief StructTest::initTestCase
 */
void StructTest::initTestCase()
{

}

/**
 * @brief StructTest::testEdgeForwardSection
 */
void StructTest::testEdgeForwardSection()
{
    KyoChanForwardEdgeSection testSection;

    testSection.forwardEdges[0].targetId = 1;
    UNITTEST(testSection.updateWeight(0, 10.0f), true);
    UNITTEST(testSection.updateWeight(17, 10.0f), false);

    UNITTEST(testSection.totalWeight, 10.0f);
}

/**
 * @brief StructTest::testEdgeSection
 */
void StructTest::testEdgeSection()
{
    KyoChanEdgeSection testSection;
    KyoChanEdge testEdge;

    UNITTEST(testSection.addEdge(testEdge), true);
    UNITTEST(testSection.isFull(), false);

    UNITTEST(testSection.updateWeight(0, 10.0f), true);
    UNITTEST(testSection.updateWeight(1, 10.0f), false);

    UNITTEST(testSection.totalWeight, 10.0f);

    for(int i = 0; i < EDGES_PER_EDGESECTION - 1; i++) {
        testSection.addEdge(testEdge);
    }

    UNITTEST(testSection.addEdge(testEdge), false);
    UNITTEST(testSection.isFull(), true);
}

/**
 * @brief StructTest::cleanupTestCase
 */
void StructTest::cleanupTestCase()
{

}

}
