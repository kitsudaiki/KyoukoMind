/**
 *  @file    globalValues.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef GLOBALVALUES_H
#define GLOBALVALUES_H

#include <common.h>

struct GlobalValues
{
    float globalMemorizingOffset = 0.0f;
    float globalMemorizingTemp = 0.0f;

    float globalLearningOffset = 0.0f;
    float globalLearningTemp = 0.0f;

    float globalOutLearningOffset = 0.0f;
    float globalOutLearningTemp = 0.0f;

    float globalGlia = 1.0f;
};

#endif // GLOBALVALUES_H
