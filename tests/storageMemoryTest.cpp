/**
 *  @file    storageMemoryTest.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "storageMemoryTest.h"

namespace KyoukoMind
{

/**
 * @brief StorageMemoryTest::StorageMemoryTest
 */
StorageMemoryTest::StorageMemoryTest() : CommonTest("StorageMemoryTest")
{
    initTestCase();
    resizeFile();
    getSizeOfFile();
    writeFile();
    readFile();
    reopenFile();
    cleanupTestCase();
}

/**
 * @brief StorageMemoryTest::aligned_malloc
 * @param size
 * @return
 */
void* StorageMemoryTest::aligned_malloc(const uint32_t size)
{
    void *mem = malloc(size+4096+sizeof(void*));
    void **ptr = (void**)((uintptr_t)(mem+4096+sizeof(void*)) & ~(4096-1));
    ptr[-1] = mem;
    memset(ptr, 0, size);
    return ptr;
}

/**
 * @brief StorageMemoryTest::aligned_free
 * @param ptr
 * @return
 */
bool StorageMemoryTest::aligned_free(void *ptr)
{
    if(ptr != nullptr) {
        free(((void**)ptr)[-1]);
        ptr = nullptr;
        return true;
    }
    return false;
}

/**
 * @brief StorageMemoryTest::initTestCase
 */
void StorageMemoryTest::initTestCase()
{
    m_storage = new PerformanceIO::StorageMemory("/tmp/cluster0");
    m_buffer = aligned_malloc(4096);
}

/**
 * @brief StorageMemoryTest::resizeFile
 */
void StorageMemoryTest::resizeFile()
{
    UNITTEST(m_storage->allocateMemory(0), true);
    UNITTEST(m_storage->allocateMemory(123), false);
    UNITTEST(m_storage->allocateMemory(16*4096), true);
}

/**
 * @brief StorageMemoryTest::getSizeOfFile
 */
void StorageMemoryTest::getSizeOfFile()
{
    UNITTEST((int)m_storage->getFileSize(false), 16*4096);
    UNITTEST((int)m_storage->getFileSize(true), 16*4096);
}

/**
 * @brief StorageMemoryTest::writeFile
 */
void StorageMemoryTest::writeFile()
{
    ((uint8_t*)m_buffer)[0] = 7;
    ((uint8_t*)m_buffer)[16] = 2;
    UNITTEST(m_storage->writeBlock(0, m_buffer, 123,0),false);
    UNITTEST(m_storage->writeBlock(0, nullptr, 123,0), false);
    UNITTEST(m_storage->writeBlock(0, m_buffer, 4096,0), true);
    UNITTEST(m_storage->writeBlock(2*4096, m_buffer, 4096,0), true);
}

/**
 * @brief StorageMemoryTest::readFile
 */
void StorageMemoryTest::readFile()
{
    memset(m_buffer, 0, 4096);
    UNITTEST(m_storage->readBlock(0, nullptr, 4096,0), false);
    UNITTEST(m_storage->readBlock(0, m_buffer, 123,0), false);
    UNITTEST(m_storage->readBlock(0, m_buffer, 4096,0), true);

    UNITTEST((int)((uint8_t*)m_buffer)[0], 7);
    UNITTEST((int)((uint8_t*)m_buffer)[16], 2);

    memset(m_buffer, 0, 4096);
    UNITTEST(m_storage->readBlock(2*4096, m_buffer, 4096,0), true);
    UNITTEST((int)((uint8_t*)m_buffer)[0], 7);
    UNITTEST((int)((uint8_t*)m_buffer)[16], 2);
}

/**
 * @brief StorageMemoryTest::reopenFile
 */
void StorageMemoryTest::reopenFile()
{
    UNITTEST(m_storage->closeFile(), true);
    delete m_storage;
    m_storage = new PerformanceIO::StorageMemory("/tmp/cluster0");

    UNITTEST(m_storage->readBlock(0, nullptr, 4096,0), false);
    UNITTEST(m_storage->readBlock(0, m_buffer, 123,0), false);
    UNITTEST(m_storage->readBlock(0, m_buffer, 4096,0), true);

    UNITTEST((int)((uint8_t*)m_buffer)[0], 7);
    UNITTEST((int)((uint8_t*)m_buffer)[16], 2);
}

/**
 * @brief StorageMemoryTest::cleanupTestCase
 */
void StorageMemoryTest::cleanupTestCase()
{
    delete m_storage;
    remove("/tmp/cluster0");
}

}
