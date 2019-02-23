/**
 *  @file    weightmap.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef WEIGHTMAP_H
#define WEIGHTMAP_H

#include <core/structs/clusterMeta.h>
#include <common.h>
#include <common/methods.h>

/**
 * @brief refillWeightMap fill the weight-map which is required for learing-process
 * @param initialSide cluster-side where the message comes in
 * @param neighbors pointer to the neighbor-list to check which neighbors are initialized
 * @param weightMap pointer to the map which should be refilled
 */
inline void
refillWeightMap(const uint8_t initialSide,
                Neighbor* neighbors,
                float* weightMap,
                const uint32_t fakeRandValue = 0)
{
    std::vector<uint8_t> possibleSides;

    // cleara existing map
    for(uint8_t side = 0; side < 17; side++)
    {
        weightMap[side] = 0.0;
    }

    uint8_t runs = 10;

    // get possible next
    for(uint8_t side = 0; side < 17; side++)
    {
        if(neighbors[side].targetClusterId != UNINIT_STATE_32
                && side != initialSide) {
            possibleSides.push_back(side);
            if(side == 16) {
                weightMap[side] += 0.1;
                runs -= 1;
            }
            if(side == 8) {
                weightMap[8] += 0.2;
                runs -= 2;
            }
        }
    }

    // share weights
    for(uint8_t i = 0; i < runs; i++)
    {
        const uint8_t side = possibleSides[randomValue(fakeRandValue) % possibleSides.size()];
        weightMap[side] += 0.1;
    }
}

#endif // WEIGHTMAP_H
