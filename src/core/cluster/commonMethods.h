#ifndef COMMONMETHODS_H
#define COMMONMETHODS_H

#include <common.h>
#include <core/structs/clusterMeta.h>

struct PossibleNext
{
    bool good = true;
    uint8_t side = 0;
    Neighbor neighbor;
    uint8_t probability = 33;
};

/**
 * @brief getNext
 * @param allNeighbors
 * @param initialSite
 * @return
 */
uint8_t getNext(Neighbor* allNeighbors,
                const uint8_t initialSite,
                bool whichoutProbability = false)
{
    srand(time(NULL));
    std::vector<PossibleNext> result(3, PossibleNext);

    // get possible neighbors
    switch((int)initialSite) {
    case 0:
        result[0].neighbor = allNeighbors[2];
        result[0].side = 2;
        result[1].neighbor = allNeighbors[8];
        result[1].side = 8;
        result[2].neighbor = allNeighbors[9];
        result[2].side = 9;
        break;
    case 1:
        result[0].neighbor = allNeighbors[7];
        result[0].side = 7;
        result[1].neighbor = allNeighbors[8];
        result[1].side = 8;
        result[2].neighbor = allNeighbors[9];
        result[2].side = 9;
        break;
    case 2:
        result[0].neighbor = allNeighbors[0];
        result[0].side = 0;
        result[1].neighbor = allNeighbors[7];
        result[1].side = 7;
        result[2].neighbor = allNeighbors[8];
        result[2].side = 8;
        break;
    case 7:
        result[0].neighbor = allNeighbors[1];
        result[0].side = 1;
        result[1].neighbor = allNeighbors[2];
        result[1].side = 2;
        result[2].neighbor = allNeighbors[9];
        result[2].side = 9;
        break;
    case 8:
        result[0].neighbor = allNeighbors[0];
        result[0].side = 0;
        result[1].neighbor = allNeighbors[1];
        result[1].side = 1;
        result[2].neighbor = allNeighbors[2];
        result[2].side = 2;
        break;
    case 9:
        result[0].neighbor = allNeighbors[7];
        result[0].side = 7;
        result[1].neighbor = allNeighbors[0];
        result[1].side = 0;
        result[2].neighbor = allNeighbors[1];
        result[2].side = 1;
        break;
    default:
        break;
    }

    uint32_t totalDistance = 0;
    // check types
    if(result[0].neighbor.neighborType == EMPTYCLUSTER) {
        result[0].good = false;
    } else { totalDistance+= result[0].neighbor.distantToNextNodeCluster; }
    if(result[1].neighbor.neighborType == EMPTYCLUSTER) {
        result[1].good = false;
    } else { totalDistance+= result[1].neighbor.distantToNextNodeCluster; }
    if(result[2].neighbor.neighborType == EMPTYCLUSTER) {
        result[2].good = false;
    } else { totalDistance+= result[2].neighbor.distantToNextNodeCluster; }

    //calculate possebility
    if(!whichoutProbability) {
        if(result[0].good)
            result[0].probability =
                100 - (uint8_t)(((float)result[0].neighbor.distantToNextNodeCluster / (float)totalDistance) * 100.0);
        if(result[1].good)
            result[1].probability =
                100 - (uint8_t)(((float)result[1].neighbor.distantToNextNodeCluster / (float)totalDistance) * 100.0);
        if(result[2].good)
            result[2].probability =
                100 - (uint8_t)(((float)result[2].neighbor.distantToNextNodeCluster / (float)totalDistance) * 100.0);
    }

    // choose a side
    int randVal = rand() % 100;
    uint8_t probability = result[0].probability;
    if(result[0].good && probability <= randVal) {
        return result[0].side;
    }
    probability += result[1].probability;
    if(result[1].good && probability <= randVal) {
        return result[1].side;
    }
    probability += result[2].probability;
    if(result[2].good && probability <= randVal) {
        return result[2].side;
    }
    return 0xFF;
}

#endif // COMMONMETHODS_H
