/**
 *  @file    edges.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef EDGES_H
#define EDGES_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct Edge
{
    float weight = 0.0;
    uint32_t targetId = UNINIT_STATE_32;

} __attribute__((packed));

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

struct EdgeSection
{
    uint8_t status = ACTIVE_SECTION;
    Edge edges[25];
    float totalWeight = 0.0000001f;
    uint8_t sourceSide = 0;
    uint32_t sourceId = UNINIT_STATE_32;

    EdgeSection()
    {
        for(uint32_t i = 0; i < 25; i++)
        {
            Edge newEdge;
            edges[i] = newEdge;
        }
    }

    /**
     * summarize all sides of the edge-section
     *
     * @return the total weight of the section
     */
    float
    getTotalWeight()
    {
        float result = 0.0000001f;
        for(uint32_t i = 0; i < 25; i++)
        {
            assert(edges[i].weight >= 0.0f);
            result += edges[i].weight;
        }
        return result;
    }

    /**
     * count the active sides of the section
     *
     * @return number of active edges in the section
     */
    uint8_t
    getActiveEdges()
    {
        uint8_t count = 0;
        for(int i = 0; i < 25; i++)
        {
            if(edges[i].targetId != UNINIT_STATE_32
                    || edges[i].weight > 0.0f)
            {
                count++;
            }
        }
        return count;
    }

} __attribute__((packed));

//==================================================================================================

struct SynapseSection
{
    uint8_t status = ACTIVE_SECTION;
    uint32_t numberOfSynapses = 0;
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

    /**
     * summarize all sides of the synapse-section
     *
     * @return the total weight of the section
     */
    float
    getTotalWeight()
    {
        float result = 0.0000001f;
        for(uint32_t i = 0; i < numberOfSynapses; i++)
        {
            result += std::abs(synapses[i].weight);
        }
        return result;
    }

    /**
     * erase all synapses from the section, which are too weak
     */
    void
    makeClean()
    {
        for(uint32_t i = 0; i < numberOfSynapses; i++)
        {
            if(synapses[i].weight < 0.1f && synapses[i].weight > -0.1f)
            {
                synapses[i] = synapses[numberOfSynapses-1];
                numberOfSynapses--;
            }
        }
    }

    /**
     * check if all slots of the section are filled
     *
     * @return true, if full, else false
     */
    bool
    isFull() const
    {
        if(numberOfSynapses >= EDGES_PER_SYNAPSESECTION) {
            return true;
        }
        return false;
    }

    /**
     * add a new synapse to the current section
     *
     * @return false, if the section is already full, else true
     */
    bool
    addSynapse(const Synapse &newSynapse)
    {
        if(numberOfSynapses >= EDGES_PER_SYNAPSESECTION) {
            return false;
        }
        synapses[numberOfSynapses] = newSynapse;
        numberOfSynapses++;
        return true;
    }
} __attribute__((packed));

//==================================================================================================

} // namespace KyoukoMind

#endif // EDGES_H
