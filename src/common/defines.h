#ifndef DEFINES_H
#define DEFINES_H

#include <common/includes.h>

#define UNITTEST(IS_VAL, SHOULD_VAL) if(IS_VAL != SHOULD_VAL) \
{  \
    m_failedTests++; \
    std::cout << std::endl; \
    std::cout << "Error in Unit-Test" << std::endl; \
    std::cout << "   File: " << __FILE__ << std::endl; \
    std::cout << "   Method: " << __PRETTY_FUNCTION__ << std::endl; \
    std::cout << "   Line: " << __LINE__ << std::endl; \
    std::cout << "   Variable: " << #IS_VAL << std::endl; \
    std::cout << "   Should-Value: " << SHOULD_VAL << std::endl; \
    std::cout << "   Is-Value: " << IS_VAL << std::endl; \
    std::cout << std::endl; \
    return; \
    } else { \
    m_successfulTests++; \
    }

inline std::string methodName(const std::string& prettyFunction)
{
    size_t colons = prettyFunction.find("::");
    size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
    size_t end = prettyFunction.rfind("(") - begin;
    // TODO: fix result
    return prettyFunction.substr(begin,end) + "()";
}

#define __METHOD_NAME__ methodName(__PRETTY_FUNCTION__)


inline std::string className(const std::string& prettyFunction)
{
    size_t colons = prettyFunction.find("::");
    if (colons == std::string::npos)
        return "::";
    size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
    size_t end = colons - begin;

    return prettyFunction.substr(begin,end);
}

#define __CLASS_NAME__ className(__PRETTY_FUNCTION__)

#endif // DEFINES_H
