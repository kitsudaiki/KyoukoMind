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
    Synapse synapses[EDGES_PER_SYNAPSESECTION];

    SynapseSection()
    {
        for(uint32_t i = 0; i < EDGES_PER_SYNAPSESECTION; i++)
        {
            Synapse newSynapse;
            synapses[i] = newSynapse;
        }
    }
} __attribute__((packed));

//==================================================================================================

inline float
abs(const float input)
{
    float floatRep = input;
    uint32_t* convertedValue = (uint32_t*)(&floatRep);
    // delete sign-bit
    *convertedValue = 0x7FFFFFFF & *convertedValue;
    return *(float*)(convertedValue);
}

//==================================================================================================

/**
 * check if all slots of the section are filled
 *
 * @return true, if full, else false
 */
inline bool
isFull(SynapseSection &section)
{
    return section.numberOfSynapses >= EDGES_PER_SYNAPSESECTION;
}

//==================================================================================================

/**
 * add a new synapse to the current section
 *
 * @return false, if the section is already full, else true
 */
inline void
addSynapse(SynapseSection &section,
           const Synapse &newSynapse)
{
    const uint8_t ok = section.numberOfSynapses < EDGES_PER_SYNAPSESECTION;
    const uint32_t pos = ((ok * section.numberOfSynapses) +
                          (EDGES_PER_SYNAPSESECTION-1))
                         % EDGES_PER_SYNAPSESECTION;

    section.synapses[pos] = newSynapse;
    section.numberOfSynapses += ok;
}

//==================================================================================================

/**
 * @brief updateSynapseWeight
 * @param section
 * @param position
 * @return
 */
inline void
updateSynapseWeight(SynapseSection &section,
                    const uint32_t position,
                    const float weightUpdate)
{
    const uint8_t ok = position < section.numberOfSynapses;
    const uint32_t pos = ((ok * position) +
                          (EDGES_PER_SYNAPSESECTION-1))
                         % EDGES_PER_SYNAPSESECTION;

    float diff = abs(section.synapses[pos].weight);
    section.synapses[pos].weight += weightUpdate;
    diff -= abs(section.synapses[pos].weight);
    section.totalWeight -= ok * diff;
}

//==================================================================================================

} // namespace KyoukoMind

#endif // SYNAPSES_H
