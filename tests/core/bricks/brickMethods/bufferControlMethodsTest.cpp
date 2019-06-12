/**
 *  @file    bufferControlMethodsTest.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "bufferControlMethodsTest.h"
#include <core/bricks/brickObjects/brick.h>
#include <core/bricks/brickMethods/bufferControlMethods.h>

namespace KyoukoMind
{

BufferControlMethodsTest::BufferControlMethodsTest() : Kitsune::CommonTest("BufferControlMethodsTest")
{
    testInitDataBlocks();

    testInitNodeBlocks();
    testInitEdgeSectionBlocks();
    testInitForwardEdgeSectionBlocks();

    testDeleteDynamicItem();
    testReserveDynamicItem();
    testReuseItemPosition();

    testAddEdge();
    testAddEmptyEdgeSection();
    testAddEmptyForwardEdgeSection();
}

void BufferControlMethodsTest::testInitDataBlocks()
{
    // init
    Brick testObject(0,0,0);

    // precheck
    UNITTEST(testObject.dataConnections[FORWARDEDGE_DATA].itemSize, 0);
    UNITTEST(testObject.dataConnections[FORWARDEDGE_DATA].numberOfItemBlocks, 0);

    // run test
    UNITTEST(initDataBlocks(&testObject, FORWARDEDGE_DATA, 420, 10), true);

    // postcheck
    UNITTEST(testObject.dataConnections[FORWARDEDGE_DATA].numberOfItems, 420);
    UNITTEST(testObject.dataConnections[FORWARDEDGE_DATA].numberOfItemBlocks, 2);
}

void BufferControlMethodsTest::testInitNodeBlocks()
{
    // init
    Brick testObject(0,0,0);
    srand(42);

    // run test
    UNITTEST(initNodeBlocks(&testObject, 10), true);
    UNITTEST(initNodeBlocks(&testObject, 10), false);

    // postcheck
    UNITTEST(testObject.dataConnections[NODE_DATA].numberOfItems, 10);
    Node* nodes = getNodeBlock(&testObject.dataConnections[NODE_DATA]);
    UNITTEST(nodes[4].border, 42.0f);
}

void BufferControlMethodsTest::testInitEdgeSectionBlocks()
{
    // init
    Brick testObject(0,0,0);
    srand(42);

    // run test
    UNITTEST(initEdgeSectionBlocks(&testObject, 10), true);
    UNITTEST(initEdgeSectionBlocks(&testObject, 10), false);

    // postcheck
    EdgeSection* sections = getEdgeSectionBlock(&testObject.dataConnections[EDGE_DATA]);
    UNITTEST(sections[4].sourceId, 4);
}

void BufferControlMethodsTest::testInitForwardEdgeSectionBlocks()
{
    // init
    Brick testObject(0,0,0);
    srand(42);

    // precheck


    // run test
    UNITTEST(initForwardEdgeSectionBlocks(&testObject, 10), true);
    UNITTEST(initForwardEdgeSectionBlocks(&testObject, 10), false);

    // postcheck
    UNITTEST(testObject.dataConnections[FORWARDEDGE_DATA].numberOfItems, 10);
    ForwardEdgeSection* sections = getForwardEdgeBlock(&testObject.dataConnections[FORWARDEDGE_DATA]);
    UNITTEST(sections[4].status, ACTIVE_SECTION);
}

void BufferControlMethodsTest::testDeleteDynamicItem()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initEdgeSectionBlocks(&testObject, 10);

    // precheck
    EdgeSection* sections = getEdgeSectionBlock(&testObject.dataConnections[EDGE_DATA]);
    UNITTEST(sections[4].status, ACTIVE_SECTION);

    // run test
    UNITTEST(deleteDynamicItem(&testObject, EDGE_DATA, 4), true);
    UNITTEST(deleteDynamicItem(&testObject, EDGE_DATA, 4), false);
    UNITTEST(deleteDynamicItem(&testObject, EDGE_DATA, 10), false);

    // postcheck
    sections = getEdgeSectionBlock(&testObject.dataConnections[EDGE_DATA]);
    UNITTEST(sections[4].status, DELETED_SECTION);
}

void BufferControlMethodsTest::testReuseItemPosition()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initEdgeSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, EDGE_DATA, 4);

    // precheck
    EdgeSection* sections = getEdgeSectionBlock(&testObject.dataConnections[EDGE_DATA]);
    UNITTEST(sections[4].status, DELETED_SECTION);

    // run test
    UNITTEST(reuseItemPosition(&testObject, EDGE_DATA), 4);
    UNITTEST(reuseItemPosition(&testObject, EDGE_DATA), UNINIT_STATE_32);
}

void BufferControlMethodsTest::testReserveDynamicItem()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initEdgeSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, EDGE_DATA, 4);

    // precheck
    EdgeSection* sections = getEdgeSectionBlock(&testObject.dataConnections[EDGE_DATA]);
    UNITTEST(sections[4].status, DELETED_SECTION);

    // run test
    UNITTEST(reserveDynamicItem(&testObject, EDGE_DATA), 4);
    UNITTEST(reserveDynamicItem(&testObject, EDGE_DATA), 10);
}

void BufferControlMethodsTest::testAddEdge()
{
    Brick testObject(0,0,0);
    srand(42);
    initEdgeSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, EDGE_DATA, 4);

    // run test
    Edge newEdge;
    UNITTEST(addEdge(&testObject, 1, newEdge), true);
}

void BufferControlMethodsTest::testAddEmptyEdgeSection()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initEdgeSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, EDGE_DATA, 4);

    // precheck
    EdgeSection* sections = getEdgeSectionBlock(&testObject.dataConnections[EDGE_DATA]);
    UNITTEST(sections[4].status, DELETED_SECTION);

    // run test
    UNITTEST(addEmptyEdgeSection(&testObject, 42), 4);
    UNITTEST(addEmptyEdgeSection(&testObject, 42), 10);

    // postcheck
    sections = getEdgeSectionBlock(&testObject.dataConnections[EDGE_DATA]);
    UNITTEST(sections[4].status, ACTIVE_SECTION);
}

void BufferControlMethodsTest::testAddEmptyForwardEdgeSection()
{
    // init
    Brick testObject(0,0,0);
    srand(42);
    initForwardEdgeSectionBlocks(&testObject, 10);
    deleteDynamicItem(&testObject, FORWARDEDGE_DATA, 4);

    // precheck
    ForwardEdgeSection* sections = getForwardEdgeBlock(&testObject.dataConnections[0]);
    UNITTEST(sections[4].status, DELETED_SECTION);

    // run test
    UNITTEST(addEmptyForwardEdgeSection(&testObject, 10, 42), 4);
    UNITTEST(addEmptyForwardEdgeSection(&testObject, 10, 42), 10);

    // postcheck
    sections = getForwardEdgeBlock(&testObject.dataConnections[FORWARDEDGE_DATA]);
    UNITTEST(sections[4].status, ACTIVE_SECTION);
}

}
