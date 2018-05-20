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
    uint32_t targetId = 0;
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
    uint16_t pendingEdges = 0;
    float totalWeight = 0.0;
    float memorize = 1.0;

    /**
     * @brief updateMemorize
     * @param diff
     */
    void updateMemorize(const float diff) {
        memorize += diff;
        if(memorize < 0.0f) {
            memorize = 0.0f;
        }
    }

    /**
     * @brief memorizeWeight
     */
    void memorizeWeight() {
        totalWeight *= std::tanh(memorize);
    }

    /**
     * @brief updateWeight
     * @param side
     * @param weight
     * @return
     */
    bool updateWeight(const uint8_t side, const float weight)
    {
        if(side >= 16) {
            return false;
        }
        if(forwardEdges[side].targetId == 0) {
            return false;
        }
        forwardEdges[side].weight += weight;
        totalWeight += weight;
        return true;
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

} __attribute__((packed));

/**
 * @brief The KyoChanEdgeSection struct
 */
struct KyoChanEdgeSection
{
    KyoChanEdge edges[EDGES_PER_EDGESECTION];
    uint32_t numberOfEdges = 0;
    float totalWeight = 0.0;
    float memorize = 1.0;

    /**
     * @brief updateMemorize
     * @param diff
     */
    void updateMemorize(const float diff) {
        memorize += diff;
        if(memorize < 0.0f) {
            memorize = 0.0f;
        }
    }

    /**
     * @brief memorizeWeight
     */
    void memorizeWeight() {
        totalWeight *= std::tanh(memorize);
    }

    /**
     * @brief updateWeight
     * @param pos
     * @param weight
     * @return
     */
    bool updateWeight(const uint8_t pos, const float weight)
    {
        if(pos >= numberOfEdges) {
            return false;
        }
        edges[pos].weight += weight;
        totalWeight += weight;
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
