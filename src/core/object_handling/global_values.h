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

    float initialMemorizing = 0.5f;

    float nodeCooldown = 3.0f;
    float actionPotential = 100.0f;
    uint64_t refractionTime = 2;

    float newSynapseBorder = 1.0f;
    float deleteSynapseBorder = 0.1f;
    uint32_t maxSomaDistance = 5;

    uint32_t numberOfNodesPerBrick = 1000;

    uint8_t padding[208];
};

} // namespace KyoukoMind

#endif // GLOBAL_VALUES_HANDLER_H
