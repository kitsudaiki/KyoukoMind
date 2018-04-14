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

struct KyoChanAxonEdge
{
    float weight = 0.0;
    uint32_t targetClusterPath = 0;
    uint32_t targetAxonId = 0;
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
