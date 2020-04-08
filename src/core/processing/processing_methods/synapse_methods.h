#ifndef SYNAPSE_METHODS_H
#define SYNAPSE_METHODS_H

#include <common.h>
#include <core/objects/synapses.h>

namespace KyoukoMind
{

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

}

#endif // SYNAPSE_METHODS_H
