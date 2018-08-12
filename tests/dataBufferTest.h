/**
 *  @file    dataBufferTest.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef IOBUFFERTEST_H
#define IOBUFFERTEST_H

#include <common.h>
#include <commonTest.h>
#include <files/dataBuffer.h>

namespace KyoukoMind
{

class DataBufferTest : public Kitsune::CommonTest
{

public:
    DataBufferTest();

private:
    void initTestCase();
    void getNumberOfBlocksAfterInit();
    void allocateBlocks();
    void getNumberOfBlocksAfterResize();
    void writeToBuffer();
    void readFromBuffer();
    void closeAndReopen();
    void readFromBufferAfterReopen();
    void closeAndReopenWithoutFile();
    void cleanupTestCase();

    PerformanceIO::DataBuffer *m_buffer = nullptr;
};

}

#endif // IOBUFFERTEST_H
