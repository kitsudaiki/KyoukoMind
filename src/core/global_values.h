/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef GLOBAL_VALUES_HANDLER_H
#define GLOBAL_VALUES_HANDLER_H

#include <common.h>

namespace KyoukoMind
{
struct GlobalValues
{
    float globalMemorizingOffset = 0.0f;
    float globalMemorizingTemp = 0.0f;

    float globalLearningOffset = 0.50f;
    float globalLearningTemp = 0.0f;

    float globalOutLearningOffset = 0.0f;
    float globalOutLearningTemp = 0.0f;

    float globalGlia = 1.0f;
    uint32_t runUpdate = 0;
};

} // namespace KyoukoMind

#endif // GLOBAL_VALUES_HANDLER_H
