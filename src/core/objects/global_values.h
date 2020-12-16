/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef GLOBAL_VALUES_HANDLER_H
#define GLOBAL_VALUES_HANDLER_H

#include <common.h>

struct GlobalValues
{
    uint32_t numberOfNodesPerBrick = 1000;

    float sensitivity = 0.0f;
    float lerningValue = 0.0f;
    float gliaValue = 1.0f;
    float initialMemorizing = 0.5f;
    float memorizingOffset = 0.0f;

    float nodeCooldown = 3.0f;
    float actionPotential = 100.0f;
    uint64_t refractionTime = 2;

    float newSynapseBorder = 1.0f;
    float deleteSynapseBorder = 0.1f;

    uint8_t padding[208];
};

#endif // GLOBAL_VALUES_HANDLER_H
