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

struct KyoChanAxonEdgeContainer
{
    uint8_t type = AXON_EDGE_CONTAINER;

    float weight = 0.0;

    uint32_t targetClusterPath = 0;
    uint32_t targetAxonId = 0;

    uint8_t padding[7];
} __attribute__((packed));

struct KyoChanPendingEdgeContainer
{
    uint8_t type = PENDING_EDGE_CONTAINER;

    uint32_t newEdgeId = 0;

    float weight = 0.0;
    uint16_t targetNodeId = 0;

    uint8_t validCounter = 0;
    uint8_t nextSite = 0;

    uint8_t padding[7];
} __attribute__((packed));

struct KyoChanPendingEdgeSectionSmall
{
    uint16_t numberOfPendingEdges = 0;
    KyoChanPendingEdgeContainer pendingEdges[PENDING_EDGES_PER_EDGESECTION_SMALL];
    uint8_t padding[14];

    /**
     * @brief checkPendingEdges
     */
    void checkPendingEdges()
    {
        if(numberOfPendingEdges == 0) {
            return;
        }
        KyoChanPendingEdgeContainer* end = pendingEdges + PENDING_EDGES_PER_EDGESECTION_SMALL;
        for(KyoChanPendingEdgeContainer* pendingEdge = pendingEdges;
            pendingEdge < end;
            pendingEdge++)
        {
            pendingEdge->validCounter++;
            if(pendingEdge->validCounter >= MAX_PENDING_VALID_CYCLES) {
                KyoChanPendingEdgeContainer emptyEdge;
                *pendingEdge = emptyEdge;
                numberOfPendingEdges--;
            }
        }
    }

    /**
     * @brief addPendingEdges
     * @param newEdge
     * @return
     */
    bool addPendingEdges(const KyoChanPendingEdgeContainer &newEdge)
    {
        if(numberOfPendingEdges == PENDING_EDGES_PER_EDGESECTION_SMALL) {
            return false;
        }
        KyoChanPendingEdgeContainer* end = pendingEdges + PENDING_EDGES_PER_EDGESECTION_SMALL;
        for(KyoChanPendingEdgeContainer* pendingEdge = pendingEdges;
            pendingEdge < end;
            pendingEdge++)
        {
            if(pendingEdge->newEdgeId == 0) {
                *pendingEdge = newEdge;
                return true;
            }
        }
        return false;
    }

} __attribute__((packed));

struct KyoChanPendingEdgeSectionBig
{
    uint16_t numberOfPendingEdges = 0;
    KyoChanPendingEdgeContainer pendingEdges[PENDING_EDGES_PER_EDGESECTION_BIG];
    uint8_t padding[14];

    /**
     * @brief checkPendingEdges
     */
    void checkPendingEdges()
    {
        OUTPUT("---")
        OUTPUT("checkPendingEdges")
        if(numberOfPendingEdges == 0) {
            return;
        }
        KyoChanPendingEdgeContainer* end = pendingEdges + PENDING_EDGES_PER_EDGESECTION_BIG;
        for(KyoChanPendingEdgeContainer* pendingEdge = pendingEdges;
            pendingEdge < end;
            pendingEdge++)
        {
            pendingEdge->validCounter++;
            if(pendingEdge->validCounter >= MAX_PENDING_VALID_CYCLES) {
                KyoChanPendingEdgeContainer emptyEdge;
                *pendingEdge = emptyEdge;
                numberOfPendingEdges--;
            }
        }
    }

    /**
     * @brief addPendingEdges
     * @param newEdge
     * @return
     */
    bool addPendingEdges(const KyoChanPendingEdgeContainer &newEdge)
    {
        if(numberOfPendingEdges == PENDING_EDGES_PER_EDGESECTION_BIG) {
            return false;
        }
        KyoChanPendingEdgeContainer* end = pendingEdges + PENDING_EDGES_PER_EDGESECTION_BIG;
        for(KyoChanPendingEdgeContainer* pendingEdge = pendingEdges;
            pendingEdge < end;
            pendingEdge++)
        {
            if(pendingEdge->newEdgeId == 0) {
                *pendingEdge = newEdge;
                return true;
            }
        }
        return false;
    }

} __attribute__((packed));

struct KyoChanEdgeContainer
{
    uint8_t type = EDGE_CONTAINER;

    float weight = 0.0;

    uint32_t targetClusterPath = 0;
    uint16_t targetNodeId = 0;

    uint64_t padding1 = 0;
    uint8_t padding2 = 0;
} __attribute__((packed));

struct KyoChanEdge
{
    float weight = 0.0;

    uint32_t targetClusterPath = 0;
    uint16_t targetNodeId = 0;
} __attribute__((packed));


struct KyoChanEdgeSection
{
    uint16_t numberOfEdges = 0;
    KyoChanEdge edges[EDGES_PER_EDGESECTION];

    /**
     * @brief isFull
     * @return
     */
    bool isFull() const {
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
    bool addEdge(const KyoChanEdge &newEdge) {
        if(numberOfEdges >= EDGES_PER_EDGESECTION) {
            return false;
        }
        edges[numberOfEdges] = newEdge;
        numberOfEdges++;
        return true;
    }
} __attribute__((packed));


#endif // KYOCHANEDGES_H
