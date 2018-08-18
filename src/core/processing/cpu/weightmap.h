#ifndef WEIGHTMAP_H
#define WEIGHTMAP_H

#include <common.h>
#include <core/structs/clusterMeta.h>

namespace KyoukoMind
{

/**
 * @brief refillWeightMap
 * @param initialSide
 * @param neighbors
 */
inline void refillWeightMap(const uint8_t initialSide, Neighbor *neighbors, float* weightMap)
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
        if(neighbors[side].targetClusterId != UNINIT_STATE
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
        const uint8_t side = possibleSides[rand() % possibleSides.size()];
        weightMap[side] += 0.1;
    }
}

/**
 * @brief randFloat
 * @param b
 * @return
 */
inline float randFloat(const float b)
{
    const float random = ((float) rand()) / (float) UNINIT_STATE;
    return random * b;
}

}

#endif // WEIGHTMAP_H
