#ifndef COMMONMETHODS_H
#define COMMONMETHODS_H

#include <common.h>
#include <core/structs/clusterMeta.h>

struct PossibleNext
{
    bool good = true;
    uint8_t side = 0;
    Neighbor neighbor;
    uint8_t probability = 0;
};

/**
 * @brief getPosibleNext
 * @param allNeighbors
 * @param initialSite
 * @return
 */
std::vector<PossibleNext> getPosibleNext(Neighbor* allNeighbors,
                                         const uint8_t initialSite)
{
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

    // check types
    if(result[0].neighbor.neighborType == EMPTYCLUSTER) {
        result[0].good = false;
    }
    if(result[1].neighbor.neighborType == EMPTYCLUSTER) {
        result[1].good = false;
    }
    if(result[2].neighbor.neighborType == EMPTYCLUSTER) {
        result[2].good = false;
    }

    //calculate possebility
    uint32_t totalDistance = result[0].neighbor.distantToNextNodeCluster
            + result[1].neighbor.distantToNextNodeCluster
            + result[2].neighbor.distantToNextNodeCluster;

    result[0].probability = (uint32_t)(((float)result[0].neighbor.distantToNextNodeCluster / (float)totalDistance) * 100.0);
    result[1].probability = (uint32_t)(((float)result[1].neighbor.distantToNextNodeCluster / (float)totalDistance) * 100.0);
    result[2].probability = (uint32_t)(((float)result[2].neighbor.distantToNextNodeCluster / (float)totalDistance) * 100.0);

    return result;
}

#endif // COMMONMETHODS_H
