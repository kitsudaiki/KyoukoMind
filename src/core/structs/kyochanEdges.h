/**
 *  @file    kyochanEdges.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef KYOCHANEDGES_H
#define KYOCHANEDGES_H

#include <common.h>

/**
 * @brief The KyoChanForwardEdge struct
 */
struct KyoChanForwardEdge
{
    float weight = 0.0;
    uint32_t targetId = UNINIT_STATE_32;

} __attribute__((packed));

/**
 * @brief The KyoChanEdge struct
 */
struct KyoChanEdge
{
    float weight = 0.0;
    uint16_t targetNodeId = UNINIT_STATE_16;
    float memorize = INITIAL_MEMORIZING;
    uint8_t inProcess = 0;
    uint8_t somaDistance = 1;

} __attribute__((packed));

/**
 * @brief The KyoChanForwardEdgeSection struct
 */
struct KyoChanForwardEdgeSection
{
    uint8_t status = ACTIVE_SECTION;
    KyoChanForwardEdge forwardEdges[17];
    //uint8_t numberOfActiveEdges = 0;
    float totalWeight = 0.0000001f;
    uint8_t sourceSide = 0;
    uint32_t sourceId = UNINIT_STATE_32;

    /**
     * @brief KyoChanForwardEdgeSection
     */
    KyoChanForwardEdgeSection()
    {
        for(uint32_t i = 0; i < 17; i++) {
            KyoChanForwardEdge newEdge;
            forwardEdges[i] = newEdge;
        }
    }

    float getTotalWeight() {
        float result = 0.0000001f;
        for(uint32_t i = 0; i < 17; i++) {
            result += std::abs(forwardEdges[i].weight);
        }
        return result;
    }

    uint8_t getActiveEdges()
    {
        uint8_t count = 0;
        for(int i = 0; i < 17; i++)
        {
            if(forwardEdges[i].targetId != UNINIT_STATE_32
                    || forwardEdges[i].weight > 0.0f) {
                count++;
            }
        }
        return count;
    }

} __attribute__((packed));

/**
 * @brief The KyoChanEdgeSection struct
 */
struct KyoChanEdgeSection
{
    uint8_t status = ACTIVE_SECTION;
    KyoChanEdge edges[EDGES_PER_EDGESECTION];
    uint32_t numberOfEdges = 0;
    uint8_t sourceSide = 0;
    uint32_t sourceId = UNINIT_STATE_32;

    /**
     * @brief KyoChanEdgeSection
     */
    KyoChanEdgeSection()
    {
        for(uint32_t i = 0; i < EDGES_PER_EDGESECTION; i++) {
            KyoChanEdge newEdge;
            edges[i] = newEdge;
        }
    }

    /**
     * @brief getTotalWeight
     * @return
     */
    float getTotalWeight() {
        float result = 0.0000001f;
        for(uint32_t i = 0; i < numberOfEdges; i++) {
            result += std::abs(edges[i].weight);
        }
        return result;
    }

    /**
     * @brief makeClean
     */
    void makeClean()
    {
        for(uint32_t i = 0; i < numberOfEdges; i++)
        {
            if(edges[i].weight < 0.1f && edges[i].weight > -0.1f)
            {
                edges[i] = edges[numberOfEdges-1];
                numberOfEdges--;
            }
        }
    }

    /**
     * @brief isFull
     * @return
     */
    bool isFull() const
    {
        if(numberOfEdges >= EDGES_PER_EDGESECTION) {
            return true;
        }
        return false;
    }

    /**
     * @brief addEdge
     * @param newEdge
     * @return
     */
    bool addEdge(const KyoChanEdge &newEdge)
    {
        if(numberOfEdges >= EDGES_PER_EDGESECTION) {
            return false;
        }
        edges[numberOfEdges] = newEdge;
        numberOfEdges++;
        return true;
    }
} __attribute__((packed));


#endif // KYOCHANEDGES_H
