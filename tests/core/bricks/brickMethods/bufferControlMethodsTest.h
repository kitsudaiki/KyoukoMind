/**
 *  @file    bufferControlMethodsTest.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef BUFFERCONTROLMETHODSTEST_H
#define BUFFERCONTROLMETHODSTEST_H

#include <common.h>
#include <testing/commonTest.h>

namespace KyoukoMind
{

class BufferControlMethodsTest : public Kitsune::CommonTest
{
public:
    BufferControlMethodsTest();

private:
    void testInitDataBlocks();

    void testReuseItemPosition();
    void testReserveDynamicItem();
    void testDeleteDynamicItem();

    void testInitNodeBlocks();
    void testInitSynapseSectionBlocks();
    void testInitEdgeSectionBlocks();

    void testAddEdge();
    void testAddEmptySynapseSection();
    void testAddEmptyEdgeSection();
};

}

#endif // BUFFERCONTROLMETHODSTEST_H
