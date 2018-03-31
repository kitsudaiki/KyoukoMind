#ifndef DEFINES_H
#define DEFINES_H

#define UNITTEST(IS_VAL, SHOULD_VAL) if(IS_VAL != SHOULD_VAL) \
{  \
    std::cout << "Error in Unit-Test" << std::endl; \
    std::cout << "   File: " << __FILE__ << std::endl; \
    std::cout << "   Method: " << __PRETTY_FUNCTION__ << std::endl; \
    std::cout << "   Line: " << __LINE__ << std::endl; \
    std::cout << "   Variable: " << #IS_VAL << std::endl; \
    std::cout << "   Should-Value: " << SHOULD_VAL << std::endl; \
    std::cout << "   Is-Value: " << IS_VAL << std::endl; \
    return; \
    }

#endif // DEFINES_H
