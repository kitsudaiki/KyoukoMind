/**
 *  @file    buffer_control_methods_test.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "buffer_control_methods_test.h"
#include <core/bricks/brick_objects/brick.h>
#include <core/bricks/brick_methods/buffer_control_methods.h>

namespace KyoukoMind
{

BufferControlMethodsTest::BufferControlMethodsTest()
    : Kitsunemimi::Common::Test("BufferControlMethodsTest")
{
    initDataBlocks_test();

    initNodeBlocks_test();
    initEdgeSectionBlocks_test();
    initEdgeSectionBlocks_test();

    deleteDynamicItem_test();
    reserveDynamicItem_test();
    reuseItemPosition_test();

    addEdge_test();
    addEmptyEdgeSection_test();
    addEmptyEdgeSection_test();
}

/**
 * @brief initDataBlocks_test
 */
void
BufferControlMethodsTest::initDataBlocks_test()
{
    // init
    Brick testObject(0,0,0);

    // precheck
    TEST_EQUAL(testObject.dataConnections[EDGE_DATA].itemSize, 0);
    TEST_EQUAL(testObject.dataConnections[EDGE_DATA].numberOfItemBlocks, 0);

    // run test
    TEST_EQUAL(initDataBlocks(&testObject, EDGE_DATA, 420, 10), true);

    // postcheck
    TEST_EQUAL(testObject.dataConnections[EDGE_DATA].numberOfItems, 420);
    TEST_EQUAL(testObject.dataConnections[EDGE_DATA].numberOfItemBlocks, 2);
}

/**
 * @brief initNodeBlocks_test
 */
void
BufferControlMethodsTest::initNodeBlocks_test()
{
    // init
    Brick testObject(0,0,0);
    srand(42);

    // run test
    TEST_EQUAL(initNodeBlocks(&testObject, 10), true);
    TEST_EQUAL(initNodeBlocks(&testObject, 10), false);

    // postcheck
    TEST_EQUAL(testObject.dataConnections[NODE_DATA].numberOfItems, 10);
    Node* nodes = getNodeBlock(&testObject.dataConnections[NODE_DATA]);
    TEST_EQUAL(nodes[4].border, 42.0f);
}

/**
 * @brief initSynapseSectionBlocks_test
 */
void
BufferControlMethodsTest::initSynapseSectionBlocks_test()
{
    // init
    Brick testObject(0,0,0);
    srand(42);

    // run test
    TEST_EQUAL(initSynapseSectionBlocks(&testObject, 10), true);
    TEST_EQUAL(initSynapseSectionBlocks(&testObject, 10), false);

    // postcheck
    SynapseSection* sections = getSynapseSectionBlock(&testObject.dataConnections[SYNAPSE_DATA]);
    TEST_EQUAL(sections[4].sourceId, 4);
}

/**
 * @brief initEdgeSectionBlocks_test
 */
void
BufferControlMethodsTest::initEdgeSectionBlocks_test()
{
    // init
    Brick testObject(0,0,0);
    srand(42);

    // precheck


    // run test
    TEST_EQUAL(initEdgeSectionBlocks(&testObject, 10), true);
    TEST_EQUAL(initEdgeSectionBlocks(&testObject, 10), false);

    // postcheck
    TEST_EQUAL(testObject.dataConnections[EDGE_DATA].numberOfItems, 10);
    EdgeSection* sections = getEdgeBlock(&testObject.dataConnections[EDGE_DATA]);
    TEST_EQUAL(sections[4].status, ACTIVE_SECTION);
}

/**
 * @brief deleteDynamicItem_test
 */
void
BufferControlMethodsTest::deleteDynamicItem_test()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initSynapseSectionBlocks(&testObject, 10);

    // precheck
    SynapseSection* sections = getSynapseSectionBlock(&testObject.dataConnections[SYNAPSE_DATA]);
    TEST_EQUAL(sections[4].status, ACTIVE_SECTION);

    // run test
    TEST_EQUAL(deleteDynamicItem(&testObject, SYNAPSE_DATA, 4), true);
    TEST_EQUAL(deleteDynamicItem(&testObject, SYNAPSE_DATA, 4), false);
    TEST_EQUAL(deleteDynamicItem(&testObject, SYNAPSE_DATA, 10), false);

    // postcheck
    sections = getSynapseSectionBlock(&testObject.dataConnections[SYNAPSE_DATA]);
    TEST_EQUAL(sections[4].status, DELETED_SECTION);
}

/**
 * @brief reuseItemPosition_test
 */
void
BufferControlMethodsTest::reuseItemPosition_test()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initSynapseSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, SYNAPSE_DATA, 4);

    // precheck
    SynapseSection* sections = getSynapseSectionBlock(&testObject.dataConnections[SYNAPSE_DATA]);
    TEST_EQUAL(sections[4].status, DELETED_SECTION);

    // run test
    TEST_EQUAL(reuseItemPosition(&testObject, SYNAPSE_DATA), 4);
    TEST_EQUAL(reuseItemPosition(&testObject, SYNAPSE_DATA), UNINIT_STATE_32);
}

/**
 * @brief reserveDynamicItem_test
 */
void
BufferControlMethodsTest::reserveDynamicItem_test()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initSynapseSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, SYNAPSE_DATA, 4);

    // precheck
    SynapseSection* sections = getSynapseSectionBlock(&testObject.dataConnections[SYNAPSE_DATA]);
    TEST_EQUAL(sections[4].status, DELETED_SECTION);

    // run test
    TEST_EQUAL(reserveDynamicItem(&testObject, SYNAPSE_DATA), 4);
    TEST_EQUAL(reserveDynamicItem(&testObject, SYNAPSE_DATA), 10);
}

/**
 * @brief addEdge_test
 */
void
BufferControlMethodsTest::addEdge_test()
{
    Brick testObject(0,0,0);
    srand(42);
    initSynapseSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, SYNAPSE_DATA, 4);

    // run test
    Synapse newEdge;
    TEST_EQUAL(addSynapse(&testObject, 1, newEdge), true);
}

/**
 * @brief addEmptySynapseSection_test
 */
void
BufferControlMethodsTest::addEmptySynapseSection_test()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initSynapseSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, SYNAPSE_DATA, 4);

    // precheck
    SynapseSection* sections = getSynapseSectionBlock(&testObject.dataConnections[SYNAPSE_DATA]);
    TEST_EQUAL(sections[4].status, DELETED_SECTION);

    // run test
    TEST_EQUAL(addEmptySynapseSection(&testObject, 42), 4);
    TEST_EQUAL(addEmptySynapseSection(&testObject, 42), 10);

    // postcheck
    sections = getSynapseSectionBlock(&testObject.dataConnections[SYNAPSE_DATA]);
    TEST_EQUAL(sections[4].status, ACTIVE_SECTION);
}

/**
 * @brief addEmptyEdgeSection_test
 */
void
BufferControlMethodsTest::addEmptyEdgeSection_test()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initEdgeSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, EDGE_DATA, 4);

    // precheck
    EdgeSection* sections = getEdgeBlock(&testObject.dataConnections[0]);
    TEST_EQUAL(sections[4].status, DELETED_SECTION);

    // run test
    TEST_EQUAL(addEmptyEdgeSection(&testObject, 10, 42), 4);
    TEST_EQUAL(addEmptyEdgeSection(&testObject, 10, 42), 10);

    // postcheck
    sections = getEdgeBlock(&testObject.dataConnections[EDGE_DATA]);
    TEST_EQUAL(sections[4].status, ACTIVE_SECTION);
}

} // namespace KyoukoMind
