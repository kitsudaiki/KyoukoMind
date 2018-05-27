/**
 *  @file    nextChooser.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "nextChooser.h"

namespace KyoukoMind
{

/**
 * @brief NextChooser::NextChooser
 */
NextChooser::NextChooser()
{
}

/**
 * @brief NextChooser::getNextCluster
 * @param allNeighbors
 * @param initialSite
 * @param clusterType
 * @param whichoutProbability
 * @return
 */
uint8_t NextChooser::getNextCluster(Neighbor *allNeighbors,
                                    const uint8_t initialSite,
                                    const uint8_t clusterType,
                                    bool whichoutProbability)
{

    if(initialSite > 0xF || allNeighbors == nullptr) {
        return 0xFF;
    }

    float possibleNext[17];
    for(uint8_t i = 0; i < 17; i++) {
        possibleNext[i] = 0.0;
    }

    getPossibleNeighbors(allNeighbors, initialSite, possibleNext, clusterType);

    float totalProbability = calculatePossebilities(whichoutProbability,
                                                    possibleNext);
    if(totalProbability == 0.0) {
        return initialSite;
    }
    uint choise = chooseNeighbor(totalProbability, possibleNext);
    if(choise == 0){
        return initialSite;
    }
    return choise;
}


/**
 * @brief NextChooser::getPossibleNeighbors
 * @param allNeighbors
 * @param initialSite
 * @param possibleNext
 * @param clusterType
 */
void NextChooser::getPossibleNeighbors(Neighbor* allNeighbors,
                                       const uint8_t initialSite,
                                       float *possibleNext,
                                       const uint8_t clusterType)
{
    switch((int)initialSite)
    {
    case 0:
        if(allNeighbors[2].neighborType != EMPTY_CLUSTER) {
            possibleNext[2] = 1.0;
        }
        if(allNeighbors[3].neighborType != EMPTY_CLUSTER) {
            possibleNext[3] = 1.0;
        }
        if(allNeighbors[4].neighborType != EMPTY_CLUSTER) {
            possibleNext[4] = 1.0;
        }
        if(allNeighbors[12].neighborType != EMPTY_CLUSTER) {
            possibleNext[12] = 1.0;
        }
        if(allNeighbors[13].neighborType != EMPTY_CLUSTER) {
            possibleNext[13] = 1.0;
        }
        if(allNeighbors[14].neighborType != EMPTY_CLUSTER) {
            possibleNext[14] = 1.0;
        }
        break;
    case 2:
        possibleNext[4] = MAX_DISTANCE
                - allNeighbors[4].distantToNextNodeCluster
                + allNeighbors[4].activeNodesInNextNodeCluster;
        possibleNext[13] = MAX_DISTANCE
                - allNeighbors[13].distantToNextNodeCluster
                + allNeighbors[13].activeNodesInNextNodeCluster;
        possibleNext[14] = MAX_DISTANCE
                - allNeighbors[14].distantToNextNodeCluster
                + allNeighbors[14].activeNodesInNextNodeCluster;
        if(clusterType == NODE_CLUSTER) {
            possibleNext[8] = MAX_DISTANCE
                + allNeighbors[8].activeNodesInNextNodeCluster;
        }
        if(allNeighbors[16].targetClusterId != UNINIT_STATE) {
            possibleNext[16] = MAX_DISTANCE;
        }
        break;
    case 3:
        possibleNext[12] = MAX_DISTANCE
                - allNeighbors[12].distantToNextNodeCluster
                + allNeighbors[12].activeNodesInNextNodeCluster;
        possibleNext[13] = MAX_DISTANCE
                - allNeighbors[13].distantToNextNodeCluster
                + allNeighbors[13].activeNodesInNextNodeCluster;
        possibleNext[14] = MAX_DISTANCE
                - allNeighbors[14].distantToNextNodeCluster
                + allNeighbors[14].activeNodesInNextNodeCluster;
        if(clusterType == NODE_CLUSTER) {
            possibleNext[8] = MAX_DISTANCE
               + allNeighbors[8].activeNodesInNextNodeCluster;
        }
        if(allNeighbors[16].targetClusterId != UNINIT_STATE) {
            possibleNext[16] = MAX_DISTANCE;
        }
        break;
    case 4:
        possibleNext[2] = MAX_DISTANCE
                - allNeighbors[2].distantToNextNodeCluster
                + allNeighbors[2].activeNodesInNextNodeCluster;
        possibleNext[12] = MAX_DISTANCE
                - allNeighbors[12].distantToNextNodeCluster
                + allNeighbors[12].activeNodesInNextNodeCluster;
        possibleNext[13] = MAX_DISTANCE
                - allNeighbors[13].distantToNextNodeCluster
                + allNeighbors[13].activeNodesInNextNodeCluster;
        if(clusterType == NODE_CLUSTER) {
            possibleNext[8] = MAX_DISTANCE
                + allNeighbors[8].activeNodesInNextNodeCluster;
        }
        if(allNeighbors[16].targetClusterId != UNINIT_STATE) {
            possibleNext[16] = MAX_DISTANCE;
        }
        break;
    case 12:
        possibleNext[3] = MAX_DISTANCE
                - allNeighbors[3].distantToNextNodeCluster
                + allNeighbors[3].activeNodesInNextNodeCluster;
        possibleNext[4] = MAX_DISTANCE
                - allNeighbors[4].distantToNextNodeCluster
                + allNeighbors[4].activeNodesInNextNodeCluster;
        possibleNext[14] = MAX_DISTANCE
                - allNeighbors[14].distantToNextNodeCluster
                + allNeighbors[14].activeNodesInNextNodeCluster;
        if(clusterType == NODE_CLUSTER) {
            possibleNext[8] = MAX_DISTANCE
                + allNeighbors[8].activeNodesInNextNodeCluster;
        }
        if(allNeighbors[16].targetClusterId != UNINIT_STATE) {
            possibleNext[16] = MAX_DISTANCE;
        }
        break;
    case 13:
        possibleNext[2] = MAX_DISTANCE
                - allNeighbors[2].distantToNextNodeCluster
                + allNeighbors[2].activeNodesInNextNodeCluster;
        possibleNext[3] = MAX_DISTANCE
                - allNeighbors[3].distantToNextNodeCluster
                + allNeighbors[3].activeNodesInNextNodeCluster;
        possibleNext[4] = MAX_DISTANCE
                - allNeighbors[4].distantToNextNodeCluster
                + allNeighbors[4].activeNodesInNextNodeCluster;
        if(clusterType == NODE_CLUSTER) {
            possibleNext[8] = MAX_DISTANCE
                + allNeighbors[8].activeNodesInNextNodeCluster;
        }
        if(allNeighbors[16].targetClusterId != UNINIT_STATE) {
            possibleNext[16] = MAX_DISTANCE;
        }
        break;
    case 14:
        possibleNext[12] = MAX_DISTANCE
                - allNeighbors[12].distantToNextNodeCluster
                + allNeighbors[12].activeNodesInNextNodeCluster;
        possibleNext[2] = MAX_DISTANCE
                - allNeighbors[2].distantToNextNodeCluster
                + allNeighbors[2].activeNodesInNextNodeCluster;
        possibleNext[3] = MAX_DISTANCE
                - allNeighbors[3].distantToNextNodeCluster
                + allNeighbors[3].activeNodesInNextNodeCluster;
        if(clusterType == NODE_CLUSTER) {
            possibleNext[8] = MAX_DISTANCE
                + allNeighbors[8].activeNodesInNextNodeCluster;
        }
        if(allNeighbors[16].targetClusterId != UNINIT_STATE) {
            possibleNext[16] = MAX_DISTANCE;
        }
        break;
    default:
        break;
    }
}

/**
 * @brief NextChooser::calculatePossebilities
 * @param totalDistance
 * @return
 */
float NextChooser::calculatePossebilities(bool whichoutProbability,
                                          float *possibleNext)
{
    float totalProbability = 0.0;

    for(uint8_t i = 0; i < 17; i++)
    {
        if(whichoutProbability) {
            if(possibleNext[i] > 0.0) {
                possibleNext[i] = 1.0;
            }
        }
        totalProbability += possibleNext[i];
    }
    return totalProbability;
}

/**
 * @brief NextChooser::chooseNeighbor
 * @return
 */
uint8_t NextChooser::chooseNeighbor(const float totalProbability,
                                    float *m_possibleNext)
{
    // choose a side
    uint32_t randVal = rand() % (uint32_t)totalProbability;
    float probability = 0;
    for(uint8_t i = 0; i < 17; i++)
    {
        probability += m_possibleNext[i];
        if(probability > (float)randVal) {
            return i;
        }
    }
    return 0;
}

}
