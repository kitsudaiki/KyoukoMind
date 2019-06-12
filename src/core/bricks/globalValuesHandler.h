/**
 *  @file    globalValuesHandler.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef GLOBALVALUESHANDLER_H
#define GLOBALVALUESHANDLER_H

#include <common.h>

namespace KyoukoMind
{
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

class GlobalValuesHandler
{
public:
    GlobalValuesHandler();

    GlobalValues getGlobalValues();
    void setGlobalValues(const GlobalValues &values);

private:
    GlobalValues m_globalValues;

    std::mutex m_mutex;
};

}

#endif // GLOBALVALUESHANDLER_H
