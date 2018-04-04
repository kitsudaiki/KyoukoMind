#ifndef NETSTRUCTS_H
#define NETSTRUCTS_H

#include <common/enums.h>
#include <common/includes.h>
#include <common/defines.h>



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



struct KyoChanNode
{
    float currentState = 0;
    float border = 0;

    // Axon
    uint32_t targetClusterPath = 0;
    uint16_t targetAxonId = 0;
} __attribute__((packed));



struct KyoChanAxon
{
    float currentState = 0;

    uint32_t edgeSections[MAX_EDGESECTIONS_PER_AXON];
    uint8_t numberOfEdgeSections = 0;

    /**
     * @brief isFull
     * @return
     */
    bool isFull() const {
        if(numberOfEdgeSections >= MAX_EDGESECTIONS_PER_AXON) {
            return true;
        }
        return false;
    }

    /**
     * @brief getLastEdgeSectionPos
     * @return
     */
    uint32_t getLastEdgeSectionPos() const {
        return edgeSections[numberOfEdgeSections-1];
    }

    /**
     * @brief addEdgeSectionPos
     * @param edgeSectionPos
     * @return
     */
    bool addEdgeSectionPos(const uint32_t edgeSectionPos) {
        if(numberOfEdgeSections >= MAX_EDGESECTIONS_PER_AXON) {
            return false;
        }
        edgeSections[numberOfEdgeSections] = edgeSectionPos;
        numberOfEdgeSections++;
        return true;
    }
} __attribute__((packed));


#endif // NETSTRUCTS_H
