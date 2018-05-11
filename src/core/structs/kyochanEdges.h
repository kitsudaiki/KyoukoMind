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
    uint8_t side = 0;
    uint8_t pending = 0;
    float weight = 0.0;
    uint32_t targetEdgeSectionId = 0;
} __attribute__((packed));

/**
 * @brief The KyoChanEdge struct
 */
struct KyoChanEdge
{
    float weight = 0.0;
    uint16_t targetNodeId = 0;
} __attribute__((packed));

/**
 * @brief The KyoChanForwardEdgeSection struct
 */
struct KyoChanForwardEdgeSection
{
    KyoChanForwardEdge forwardEdges[16];
    uint8_t numberOfForwardEdges = 0;
    uint16_t pendingEdges = 0;

    /**
     * @brief flipPendingBit
     * @param pos
     */
    void flipPendingBit(const uint8_t pos)
    {
        assert(pos < 16);
        pendingEdges ^= 1UL << pos;
    }

    /**
     * @brief negatePendingBit
     * @param pos
     */
    void zeroPendingBit(const uint8_t pos)
    {
        assert(pos < 16);
        pendingEdges &= ~((uint16_t)1U << pos);
    }

    /**
     * @brief setPedingBit
     * @param pos
     */
    void setPedingBit(const uint8_t pos)
    {
        assert(pos < 16);
        pendingEdges |= (uint16_t)1U << pos;
    }

    /**
     * @brief addForwardEdge
     * @param forwardEdge
     * @return
     */
    bool addForwardEdge(const KyoChanForwardEdge &newForwardEdge)
    {
        if(numberOfForwardEdges >= EDGES_PER_EDGESECTION) {
            return false;
        }
        forwardEdges[numberOfForwardEdges] = newForwardEdge;
        numberOfForwardEdges++;
        return true;
    }
} __attribute__((packed));

/**
 * @brief The KyoChanEdgeSection struct
 */
struct KyoChanEdgeSection
{
    uint8_t incomSide = 0;
    float totalWeight = 0.0;

    KyoChanForwardEdge forwardEdges[16];
    uint8_t numberOfForwardEdges = 0;

    KyoChanEdge edges[EDGES_PER_EDGESECTION];
    uint32_t numberOfEdges = 0;

    /**
     * @brief addForwardEdge
     * @param forwardEdge
     * @return
     */
    bool addForwardEdge(const KyoChanForwardEdge &newForwardEdge)
    {
        if(numberOfForwardEdges >= EDGES_PER_EDGESECTION) {
            return false;
        }
        forwardEdges[numberOfForwardEdges] = newForwardEdge;
        numberOfForwardEdges++;
        return true;
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
