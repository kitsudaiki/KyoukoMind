#ifndef COMMONTEST_H
#define COMMONTEST_H

#include <common.h>

namespace KyoukoMind
{

class CommonTest
{
public:
    CommonTest(std::string testName);
    ~CommonTest();

protected:
    uint32_t m_successfulTests = 0;
    uint32_t m_failedTests = 0;
};

}

#endif // COMMONTEST_H
