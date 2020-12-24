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
    float actionPotential = 150.0f;
    uint64_t refractionTime = 2;

    float newSynapseBorder = 1.0f;
    float deleteSynapseBorder = 0.1f;

    float outputIndex = 0.0f;

    uint8_t padding[204];
};

#endif // GLOBAL_VALUES_HANDLER_H
