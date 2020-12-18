/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef EDGES_H
#define EDGES_H

#include <common.h>

class Brick;

//==================================================================================================

struct Edge
{
    float synapseWeight = 0.0f;
    float edgeWeight = 0.0f;
    uint32_t synapseSectionId = UNINIT_STATE_32;
    uint32_t brickLocation = UNINIT_STATE_32;
    // total size: 16 Byte
} __attribute__((packed));

inline uint32_t
getBrickId(const Edge &edge)
{
    return edge.brickLocation & 0x00FFFFFF;
}

inline uint32_t
getBrickId(const uint32_t location)
{
    return location & 0x00FFFFFF;
}

inline uint8_t
getInputSide(const uint32_t location)
{
    return location >> 24;
}

//==================================================================================================

struct EdgeSection
{
    uint32_t targetBrickId = UNINIT_STATE_32;
    uint16_t randomPos = 0;
    int16_t numberOfUsedSynapseSections = 0;

    Edge edges[255];

    uint8_t padding2[8];

    EdgeSection()
    {
        for(uint16_t i = 0; i < 255; i++)
        {
            Edge newEdge;
            edges[i] = newEdge;
        }
    }

    // total size: 4096 Byte
} __attribute__((packed));

//==================================================================================================

#endif // EDGES_H
