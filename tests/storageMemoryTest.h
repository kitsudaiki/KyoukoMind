#ifndef STORAGEMEMORYTEST_H
#define STORAGEMEMORYTEST_H

#include <common.h>
#include <tests/commonTest.h>
#include <files/storageMemory.h>

namespace KyoukoMind
{

class StorageMemoryTest : public CommonTest
{

public:
    StorageMemoryTest();

private:
    void initTestCase();
    void resizeFile();
    void getSizeOfFile();
    void writeFile();
    void readFile();
    void reopenFile();
    void cleanupTestCase();

    PerformanceIO::StorageMemory *m_storage = nullptr;
    void *m_buffer = nullptr;

    void* aligned_malloc(const uint32_t size);
    bool aligned_free(void *ptr);
};

}

#endif // STORAGEMEMORYTEST_H
