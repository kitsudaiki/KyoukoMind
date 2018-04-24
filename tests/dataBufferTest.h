#ifndef IOBUFFERTEST_H
#define IOBUFFERTEST_H

#include <common.h>
#include <tests/commonTest.h>
#include <files/dataBuffer.h>

namespace KyoukoMind
{

class DataBufferTest : public CommonTest
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
