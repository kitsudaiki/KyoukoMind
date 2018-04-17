/**
 *  @file    kyochanNodes.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef KYOCHANNODES_H
#define KYOCHANNODES_H

#include <common.h>
#include <core/structs/kyochanEdges.h>

struct KyoChanNode
{
    float currentState = 0;
    float border = 0;

    // Axon
    uint32_t targetClusterPath = 0;
    uint32_t targetAxonId = 0;
} __attribute__((packed));



struct KyoChanAxon
{
    float currentState = 0;
    KyoChanPendingEdgeSectionSmall pendingEdges;
    uint8_t numberOfPendingEdges = 0;
    uint32_t edgeSections[MAX_EDGESECTIONS_PER_AXON];
    uint8_t numberOfEdgeSections = 0;
    uint32_t numberOfEdges = 0;

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

#endif // KYOCHANNODES_H
