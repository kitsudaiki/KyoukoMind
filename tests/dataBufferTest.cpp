/**
 *  @file    dataBufferTest.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "dataBufferTest.h"

namespace KyoukoMind
{

/**
 * @brief DataBufferTest::DataBufferTest
 */
DataBufferTest::DataBufferTest() : CommonTest("DataBufferTest")
{
    initTestCase();
    getNumberOfBlocksAfterInit();
    allocateBlocks();
    getNumberOfBlocksAfterResize();
    writeToBuffer();
    readFromBuffer();
    closeAndReopen();
    readFromBufferAfterReopen();
    closeAndReopenWithoutFile();
    cleanupTestCase();
}

/**
 * @brief DataBufferTest::initTestCase
 */
void DataBufferTest::initTestCase()
{
    m_buffer = new PerformanceIO::DataBuffer("/tmp/cluster0");
}

/**
 * @brief DataBufferTest::getNumberOfBlocksAfterInit
 */
void DataBufferTest::getNumberOfBlocksAfterInit()
{
    UNITTEST((int)m_buffer->getNumberOfBlocks(), 1);
}

/**
 * @brief DataBufferTest::allocateBlocks
 */
void DataBufferTest::allocateBlocks()
{
    UNITTEST(m_buffer->allocateBlocks(10), true);
}

/**
 * @brief DataBufferTest::getNumberOfBlocksAfterResize
 */
void DataBufferTest::getNumberOfBlocksAfterResize()
{
    UNITTEST((int)m_buffer->getNumberOfBlocks(), 11);
}

/**
 * @brief DataBufferTest::writeToBuffer
 */
void DataBufferTest::writeToBuffer()
{
    uint8_t *buffer = (uint8_t*)m_buffer->getBlock(0);
    buffer[4] = 42;
    buffer[1024] = 24;
    buffer = (uint8_t*)m_buffer->getBlock(8);
    buffer[4] = 42;
    buffer[1024] = 24;
    UNITTEST(m_buffer->syncBlocks(0, 10), true);
}

/**
 * @brief DataBufferTest::readFromBuffer
 */
void DataBufferTest::readFromBuffer()
{
    uint8_t *buffer = (uint8_t*)m_buffer->getBlock(0);
    UNITTEST((int)buffer[4], 42);
    UNITTEST((int)buffer[1024], 24);
    buffer = (uint8_t*)m_buffer->getBlock(8);
    UNITTEST((int)buffer[4], 42);
    UNITTEST((int)buffer[1024], 24);
}

/**
 * @brief DataBufferTest::closeAndReopen
 */
void DataBufferTest::closeAndReopen()
{
    UNITTEST(m_buffer->closeBuffer(), true);
    UNITTEST(m_buffer->closeBuffer(), false);
    m_buffer = new PerformanceIO::DataBuffer("/tmp/cluster0");
}

/**
 * @brief DataBufferTest::readFromBufferAfterReopen
 */
void DataBufferTest::readFromBufferAfterReopen()
{
    uint8_t *buffer = (uint8_t*)m_buffer->getBlock(0);
    UNITTEST((int)buffer[4], 42);
    UNITTEST((int)buffer[1024], 24);
    buffer = (uint8_t*)m_buffer->getBlock(8);
    UNITTEST((int)buffer[4], 42);
    UNITTEST((int)buffer[1024], 24);
}

/**
 * @brief DataBufferTest::closeAndReopenWithoutFile
 */
void DataBufferTest::closeAndReopenWithoutFile()
{
    UNITTEST(m_buffer->closeBuffer(true), true);
    UNITTEST(m_buffer->initBuffer(), true);
    uint8_t *buffer = (uint8_t*)m_buffer->getBlock(0);
    UNITTEST((int)buffer[4], 42);
    UNITTEST((int)buffer[1024], 24);
    buffer = (uint8_t*)m_buffer->getBlock(8);
    UNITTEST((int)buffer[4], 42);
    UNITTEST((int)buffer[1024], 24);
}

/**
 * @brief DataBufferTest::cleanupTestCase
 */
void DataBufferTest::cleanupTestCase()
{
    delete m_buffer;
}

}
