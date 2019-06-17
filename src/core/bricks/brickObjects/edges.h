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

/**
 * @brief The Edge struct
 */
struct Edge
{
    float weight = 0.0;
    uint32_t targetId = UNINIT_STATE_32;

} __attribute__((packed));

/**
 * @brief The Edge struct
 */
struct Synapse
{
    float weight = 0.0;
    uint16_t targetNodeId = UNINIT_STATE_16;
    float memorize = INITIAL_MEMORIZING;
    uint8_t inProcess = 0;
    uint8_t somaDistance = 1;

} __attribute__((packed));

/**
 * @brief The EdgeSection struct
 */
struct EdgeSection
{
    uint8_t status = ACTIVE_SECTION;
    Edge forwardEdges[25];
    //uint8_t numberOfActiveEdges = 0;
    float totalWeight = 0.0000001f;
    uint8_t sourceSide = 0;
    uint32_t sourceId = UNINIT_STATE_32;

    /**
     * @brief EdgeSection
     */
    EdgeSection()
    {
        for(uint32_t i = 0; i < 25; i++)
        {
            Edge newEdge;
            forwardEdges[i] = newEdge;
        }
    }

    float getTotalWeight()
    {
        float result = 0.0000001f;
        for(uint32_t i = 0; i < 25; i++)
        {
            assert(forwardEdges[i].weight >= 0.0f);
            result += forwardEdges[i].weight;
        }
        return result;
    }

    uint8_t getActiveEdges()
    {
        uint8_t count = 0;
        for(int i = 0; i < 25; i++)
        {
            if(forwardEdges[i].targetId != UNINIT_STATE_32
                    || forwardEdges[i].weight > 0.0f)
            {
                count++;
            }
        }
        return count;
    }

} __attribute__((packed));

/**
 * @brief The EdgeSection struct
 */
struct SynapseSection
{
    uint8_t status = ACTIVE_SECTION;
    uint32_t numberOfSynapses = 0;
    uint32_t sourceId = UNINIT_STATE_32;
    Synapse synapses[EDGES_PER_SYNAPSESECTION];

    /**
     * @brief EdgeSection
     */
    SynapseSection()
    {
        for(uint32_t i = 0; i < EDGES_PER_SYNAPSESECTION; i++)
        {
            Synapse newSynapse;
            synapses[i] = newSynapse;
        }
    }

    /**
     * @brief getTotalWeight
     * @return
     */
    float getTotalWeight()
    {
        float result = 0.0000001f;
        for(uint32_t i = 0; i < numberOfSynapses; i++)
        {
            result += std::abs(synapses[i].weight);
        }
        return result;
    }

    /**
     * @brief makeClean
     */
    void makeClean()
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
     * @brief isFull
     * @return
     */
    bool isFull() const
    {
        if(numberOfSynapses >= EDGES_PER_SYNAPSESECTION) {
            return true;
        }
        return false;
    }

    /**
     * @brief addSynapse
     * @param newSynapse
     * @return
     */
    bool addSynapse(const Synapse &newSynapse)
    {
        if(numberOfSynapses >= EDGES_PER_SYNAPSESECTION) {
            return false;
        }
        synapses[numberOfSynapses] = newSynapse;
        numberOfSynapses++;
        return true;
    }
} __attribute__((packed));

}

#endif // EDGES_H
