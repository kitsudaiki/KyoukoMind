/**
 *  @file    methods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef METHODS_H
#define METHODS_H

#include <stdlib.h>
#include <common/defines.h>

inline uint32_t randomValue(const uint32_t fakeValue = 0) {

    #ifdef RUN_UNIT_TEST
    return fakeValue;
    #else
    return (uint32_t)rand();
    #endif
}

#endif // METHODS_H
