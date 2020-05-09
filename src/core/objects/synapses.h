#ifndef SYNAPSES_H
#define SYNAPSES_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct Synapse
{
    float weight = 0.0;
    uint16_t targetNodeId = UNINIT_STATE_16;
    float memorize = INITIAL_MEMORIZING;
    uint8_t inProcess = 0;
    uint8_t somaDistance = 1;

} __attribute__((packed));

//==================================================================================================

struct SynapseSection
{
    uint8_t status = ACTIVE_SECTION;

    uint8_t numberOfSynapses = 0;
    uint64_t activeMapping = 0;
    // has to be at least a very small value to avoid division by zero
    float totalWeight = 0.0000001f;

    uint32_t sourceId = UNINIT_STATE_32;
    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];

    SynapseSection()
    {
        for(uint32_t i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++)
        {
            Synapse newSynapse;
            synapses[i] = newSynapse;
        }
    }
} __attribute__((packed));

//==================================================================================================

} // namespace KyoukoMind

#endif // SYNAPSES_H
