/**
 *  @file    buffer_control_methods_test.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef BUFFER_CONTROL_METHODS_TEST_H
#define BUFFER_CONTROL_METHODS_TEST_H

#include <common.h>
#include <libKitsunemimiCommon/test.h>

namespace KyoukoMind
{

class BufferControlMethodsTest
        : public Kitsunemimi::Test
{
public:
    BufferControlMethodsTest();

private:
    void initDataBlocks_test();

    void reuseItemPosition_test();
    void reserveDynamicItem_test();
    void deleteDynamicItem_test();

    void initNodeBlocks_test();
    void initSynapseSectionBlocks_test();
    void initEdgeSectionBlocks_test();

    void addEdge_test();
    void addEmptySynapseSection_test();
    void addEmptyEdgeSection_test();
};

} // namespace KyoukoMind

#endif // BUFFER_CONTROL_METHODS_TEST_H
