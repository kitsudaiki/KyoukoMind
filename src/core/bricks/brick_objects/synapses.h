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

/**
 * summarize all sides of the synapse-section
 *
 * @return the total weight of the section
 */
inline float
getTotalWeight(SynapseSection &section)
{
    float result = 0.0000001f;
    for(uint32_t i = 0; i < section.numberOfSynapses; i++)
    {
        result += std::abs(section.synapses[i].weight);
    }
    return result;
}

/**
 * erase all synapses from the section, which are too weak
 */
inline void
makeClean(SynapseSection &section)
{
    for(uint32_t i = 0; i < section.numberOfSynapses; i++)
    {
        if(section.synapses[i].weight < 0.1f
                && section.synapses[i].weight > -0.1f)
        {
            section.synapses[i] = section.synapses[section.numberOfSynapses-1];
            section.numberOfSynapses--;
        }
    }
}

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

/**
 * add a new synapse to the current section
 *
 * @return false, if the section is already full, else true
 */
inline bool
addSynapse(SynapseSection &section,
           const Synapse &newSynapse)
{
    if(section.numberOfSynapses >= EDGES_PER_SYNAPSESECTION) {
        return false;
    }

    section.synapses[section.numberOfSynapses] = newSynapse;
    section.numberOfSynapses++;

    return true;
}

//==================================================================================================

} // namespace KyoukoMind

#endif // SYNAPSES_H
