/**
 * @file        kyouko_root.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef EDGES_H
#define EDGES_H

#include <common.h>

class Brick;

//==================================================================================================

struct Edge
{
    float synapseWeight = 0.0f;
    uint32_t synapseSectionId = UNINIT_STATE_32;

    uint32_t brickLocation = UNINIT_STATE_32;
    uint16_t prev = UNINIT_STATE_16;
    uint16_t next = UNINIT_STATE_16;
    float hardening = 0.0f;

    uint8_t padding[4];

    // total size: 24 Byte
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

    Edge edges[170];

    uint16_t lastPosition = 0;
    uint8_t padding2[6];

    EdgeSection()
    {
        //test();
        for(uint16_t i = 0; i < 170; i++)
        {
            Edge newEdge;
            edges[i] = newEdge;
        }
    }

    void test()
    {
        Edge test1;
        test1.brickLocation = 42;

        Edge test2;
        test2.brickLocation = 43;

        Edge test3;
        test3.brickLocation = 44;

        assert(append(test1) == 1);
        assert(append(test2) == 2);
        assert(append(test3) == 3);

        assert(edges[0].next == 1);
        assert(edges[1].next == 2);
        assert(edges[1].prev == 0);
        assert(edges[2].next == 3);
        assert(edges[2].prev == 1);
        assert(edges[3].next == UNINIT_STATE_16);
        assert(edges[3].prev == 2);

        assert(remove(2));
        assert(edges[1].next == 3);
        assert(edges[3].prev == 1);
        assert(remove(3));
        assert(edges[1].next == UNINIT_STATE_16);

        std::cout<<"success"<<std::endl;
    }

    /**
     * @brief remove
     * @param pos
     * @return
     */
    bool remove(const uint16_t pos)
    {
        if(pos >= 170
                || pos == 0)
        {
            return false;
        }

        Edge* edge = &edges[pos];
        assert(edge->brickLocation != UNINIT_STATE_32);

        Edge* prev = &edges[edge->prev];

        if(edge->next != UNINIT_STATE_16)
        {
            Edge* next = &edges[edge->next];
            next->prev = edge->prev;
            prev->next = edge->next;
        }
        else
        {
            assert(pos == lastPosition);
            prev->next = UNINIT_STATE_16;
            lastPosition = edge->prev;
        }

        Edge emptyEdge;
        edges[pos] = emptyEdge;

        return true;
    }

    /**
     * @brief append
     * @param newEdge
     * @return
     */
    uint16_t append(Edge &newEdge)
    {
        uint16_t found = UNINIT_STATE_16;
        for(uint16_t i = 1; i < 170; i++)
        {
            if(getBrickId(edges[i].brickLocation) == UNINIT_STATE_24)
            {
                found = i;
                break;
            }
        }

        if(found != UNINIT_STATE_16)
        {
            edges[found] = newEdge;
            edges[found].prev = lastPosition;
            edges[lastPosition].next = found;
            lastPosition = found;

            return found;
        }

        return UNINIT_STATE_16;
    }

    void harden(const float value)
    {
        for(uint16_t i = 1; i < 170; i++)
        {
            if(getBrickId(edges[i].brickLocation) != UNINIT_STATE_24)
            {
                const float newValue = edges[i].hardening + value;
                edges[i].hardening = (newValue > 1.0f) * 1.0f + (newValue <= 1.0f) * newValue;
            }
        }
    }

    float getTotalWeight()
    {
        float totalWeight = 0.0f;

        for(uint16_t i = 1; i < 170; i++)
        {
            if(getBrickId(edges[i].brickLocation) != UNINIT_STATE_24) {
                totalWeight += edges[i].synapseWeight;
            }
        }

        return totalWeight;
    }

    // total size: 4096 Byte
} __attribute__((packed));

//==================================================================================================

#endif // EDGES_H
