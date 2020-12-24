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
    float edgeWeight = 0.0f;
    uint16_t nextGroup = UNINIT_STATE_16;
    uint8_t padding[2];

    // total size: 8 Byte
} __attribute__((packed));

//==================================================================================================

struct EdgeGroup
{
    float synapseWeight = 0.0f;
    uint32_t synapseSectionId = UNINIT_STATE_32;

    uint32_t brickLocation = UNINIT_STATE_32;

    uint16_t prevGroup = UNINIT_STATE_16;
    uint16_t prevGroupPos = UNINIT_STATE_16;

    Edge edges[4];


    void squash()
    {
        Edge tempEdges[4];
        uint count = 0;

        for(uint i = 0; i < 4; i++)
        {
            if(edges[i].nextGroup != UNINIT_STATE_16)
            {
                tempEdges[count] = edges[i];
                count++;
            }
        }

        for(uint i = 0; i < 4 - 1; i++) {
            edges[i] = tempEdges[i];
        }
    }

    bool isEmpty()
    {
        bool result = synapseWeight == 0.0f;

        result |= edges[0].nextGroup != UNINIT_STATE_16;
        result |= edges[1].nextGroup != UNINIT_STATE_16;
        result |= edges[2].nextGroup != UNINIT_STATE_16;
        result |= edges[3].nextGroup != UNINIT_STATE_16;

        return result;
    }

    float count()
    {
        float result = synapseWeight;

        result += (edges[0].nextGroup != UNINIT_STATE_16) * edges[0].edgeWeight;
        result += (edges[1].nextGroup != UNINIT_STATE_16) * edges[1].edgeWeight;
        result += (edges[2].nextGroup != UNINIT_STATE_16) * edges[2].edgeWeight;
        result += (edges[3].nextGroup != UNINIT_STATE_16) * edges[3].edgeWeight;

        return result;
    }

    void reset()
    {
        synapseWeight = 0.0f;
        synapseSectionId = UNINIT_STATE_32;
        brickLocation = UNINIT_STATE_32;
        prevGroup = UNINIT_STATE_16;
        prevGroupPos = UNINIT_STATE_16;

        Edge emptyEdge;

        edges[0] = emptyEdge;
        edges[1] = emptyEdge;
        edges[2] = emptyEdge;
        edges[3] = emptyEdge;
    }

    uint32_t getNumberOfUninit()
    {
        uint32_t freeEdges = 0;

        freeEdges += edges[0].nextGroup == UNINIT_STATE_16;
        freeEdges += edges[1].nextGroup == UNINIT_STATE_16;
        freeEdges += edges[2].nextGroup == UNINIT_STATE_16;
        freeEdges += edges[3].nextGroup == UNINIT_STATE_16;

        return freeEdges;
    }

    // total size: 48 Byte
} __attribute__((packed));

inline uint32_t
getBrickId(const EdgeGroup &edge)
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

    EdgeGroup edgeGroups[85];

    uint8_t padding2[8];

    EdgeSection()
    {
        for(uint16_t i = 0; i < 85; i++)
        {
            EdgeGroup newEdgeGroup;
            edgeGroups[i] = newEdgeGroup;
        }
    }

    uint32_t getFreePosition()
    {
        for(uint32_t i = 0; i < 85; i++)
        {
            if(edgeGroups[i].prevGroup == UNINIT_STATE_16) {
                return i;
            }
        }

        return UNINIT_STATE_32;
    }

    // total size: 4096 Byte
} __attribute__((packed));

//==================================================================================================

#endif // EDGES_H
