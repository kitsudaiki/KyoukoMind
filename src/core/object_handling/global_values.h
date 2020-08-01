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
    float memorizingValue = 0.0f;
    float lerningValue = 0.50f;
    float gliaValue = 1.0f;

    uint8_t padding[244];
};

} // namespace KyoukoMind

#endif // GLOBAL_VALUES_HANDLER_H
