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
    KyoChanForwardEdge forwardEdges[FORWARD_EDGES_PER_EDGESECTION];
    uint8_t numberOfForwardEdges = 0;

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

    KyoChanForwardEdge forwardEdges[FORWARD_EDGES_PER_EDGESECTION];
    uint8_t numberOfForwardEdges = 0;

    KyoChanEdge edges[EDGES_PER_EDGESECTION];
    uint32_t numberOfEdges = 0;

    uint8_t padding[4];

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
