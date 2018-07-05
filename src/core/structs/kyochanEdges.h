/**
 *  @file    kyochanEdges.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
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
    uint32_t targetId = UNINIT_STATE;
    float memorize = 0.5f;

} __attribute__((packed));

/**
 * @brief The KyoChanEdge struct
 */
struct KyoChanEdge
{
    float weight = 0.0;
    uint16_t targetNodeId = (uint16_t)UNINIT_STATE;
    float memorize = 0.5f;

} __attribute__((packed));

/**
 * @brief The KyoChanForwardEdgeSection struct
 */
struct KyoChanForwardEdgeSection
{
    KyoChanForwardEdge forwardEdges[17];
    uint8_t numberOfActiveEdges = 0;
    float totalWeight = 0.0;
    uint8_t sourceSide = 0;
    uint32_t sourceId = 0;

    /**
     * @brief updateWeight
     * @param side
     * @param weight
     * @return
     */
    bool updateWeight(const uint8_t side, const float weight)
    {
        if(side < 17) {
            forwardEdges[side].weight += weight;
            totalWeight += weight;
            return true;
        }
        return false;
    }

} __attribute__((packed));

/**
 * @brief The KyoChanEdgeSection struct
 */
struct KyoChanEdgeSection
{
    KyoChanEdge edges[EDGES_PER_EDGESECTION];
    uint32_t numberOfEdges = 0;
    float totalWeight = 0.0;
    uint32_t sourceId = 0;

    /**
     * @brief updateWeight
     * @param pos
     * @param weight
     * @return
     */
    bool updateWeight(const uint8_t pos, const float weight)
    {
        if(pos < numberOfEdges) {
            edges[pos].weight += weight;
            totalWeight += weight;
            return true;
        }
        return false;
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
