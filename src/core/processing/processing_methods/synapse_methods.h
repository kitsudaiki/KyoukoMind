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
    return section.numberOfSynapses >= SYNAPSES_PER_SYNAPSESECTION;
}

//==================================================================================================

/**
 * add a new synapse to the current section
 *
 * @return false, if the section is already full, else true
 */
inline void
addSynapse(SynapseSection &section,
           const float globalMemorizingOffset,
           const uint32_t targetNodeId,
           const uint32_t somaDistance)
{
    if(section.numberOfSynapses < SYNAPSES_PER_SYNAPSESECTION)
    {
        Synapse newSynapse;

        newSynapse.targetNodeId = static_cast<uint16_t>(targetNodeId) % NUMBER_OF_NODES_PER_BRICK;
        newSynapse.memorize = globalMemorizingOffset;
        newSynapse.somaDistance = static_cast<uint8_t>(somaDistance % (MAX_SOMA_DISTANCE - 1)) + 1;

        const uint32_t pos = section.numberOfSynapses;
        section.synapses[pos] = newSynapse;
        section.numberOfSynapses++;
    }
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
    if(position < section.numberOfSynapses)
    {
        float diff = abs(section.synapses[position].weight);
        section.synapses[position].weight += weightUpdate;
        diff -= abs(section.synapses[position].weight);
        section.totalWeight -= diff;
    }
}

//==================================================================================================

}

#endif // SYNAPSE_METHODS_H
