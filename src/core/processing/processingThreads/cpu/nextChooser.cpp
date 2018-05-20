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
 * @return
 */
uint8_t NextChooser::getNextCluster(Neighbor *allNeighbors,
                                    const uint8_t initialSite,
                                    bool whichoutProbability)
{

    if(initialSite > 0xF || allNeighbors == nullptr) {
        return 0xFF;
    }

    float possibleNext[16];
    for(uint8_t i = 0; i < 16; i++) {
        possibleNext[i] = 0.0;
    }

    getPossibleNeighbors(allNeighbors, initialSite, possibleNext);

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
 */
void NextChooser::getPossibleNeighbors(Neighbor* allNeighbors,
                                       const uint8_t initialSite,
                                       float *possibleNext)
{
    switch((int)initialSite)
    {
    case 0:
        possibleNext[2] = 1.0;
        possibleNext[3] = 1.0;
        possibleNext[4] = 1.0;
        possibleNext[11] = 1.0;
        possibleNext[12] = 1.0;
        possibleNext[13] = 1.0;
        break;
    case 2:
        possibleNext[4] = MAX_DISTANCE
                - allNeighbors[4].distantToNextNodeCluster
                + allNeighbors[4].activeNodesInNextNodeCluster;
        possibleNext[12] = MAX_DISTANCE
                - allNeighbors[12].distantToNextNodeCluster
                + allNeighbors[12].activeNodesInNextNodeCluster;
        possibleNext[13] = MAX_DISTANCE
                - allNeighbors[13].distantToNextNodeCluster
                + allNeighbors[13].activeNodesInNextNodeCluster;
        break;
    case 3:
        possibleNext[11] = MAX_DISTANCE
                - allNeighbors[11].distantToNextNodeCluster
                + allNeighbors[11].activeNodesInNextNodeCluster;
        possibleNext[12] = MAX_DISTANCE
                - allNeighbors[12].distantToNextNodeCluster
                + allNeighbors[12].activeNodesInNextNodeCluster;
        possibleNext[13] = MAX_DISTANCE
                - allNeighbors[13].distantToNextNodeCluster
                + allNeighbors[13].activeNodesInNextNodeCluster;
        break;
    case 4:
        possibleNext[2] = MAX_DISTANCE
                - allNeighbors[2].distantToNextNodeCluster
                + allNeighbors[2].activeNodesInNextNodeCluster;
        possibleNext[11] = MAX_DISTANCE
                - allNeighbors[11].distantToNextNodeCluster
                + allNeighbors[11].activeNodesInNextNodeCluster;
        possibleNext[12] = MAX_DISTANCE
                - allNeighbors[12].distantToNextNodeCluster
                + allNeighbors[12].activeNodesInNextNodeCluster;
        break;
    case 11:
        possibleNext[3] = MAX_DISTANCE
                - allNeighbors[3].distantToNextNodeCluster
                + allNeighbors[3].activeNodesInNextNodeCluster;
        possibleNext[4] = MAX_DISTANCE
                - allNeighbors[4].distantToNextNodeCluster
                + allNeighbors[4].activeNodesInNextNodeCluster;
        possibleNext[13] = MAX_DISTANCE
                - allNeighbors[13].distantToNextNodeCluster
                + allNeighbors[13].activeNodesInNextNodeCluster;
        break;
    case 12:
        possibleNext[2] = MAX_DISTANCE
                - allNeighbors[2].distantToNextNodeCluster
                + allNeighbors[2].activeNodesInNextNodeCluster;
        possibleNext[3] = MAX_DISTANCE
                - allNeighbors[3].distantToNextNodeCluster
                + allNeighbors[3].activeNodesInNextNodeCluster;
        possibleNext[4] = MAX_DISTANCE
                - allNeighbors[4].distantToNextNodeCluster
                + allNeighbors[4].activeNodesInNextNodeCluster;
        break;
    case 13:
        possibleNext[11] = MAX_DISTANCE
                - allNeighbors[11].distantToNextNodeCluster
                + allNeighbors[11].activeNodesInNextNodeCluster;
        possibleNext[2] = MAX_DISTANCE
                - allNeighbors[2].distantToNextNodeCluster
                + allNeighbors[2].activeNodesInNextNodeCluster;
        possibleNext[3] = MAX_DISTANCE
                - allNeighbors[3].distantToNextNodeCluster
                + allNeighbors[3].activeNodesInNextNodeCluster;
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

    for(uint8_t i = 0; i < 16; i++)
    {
        if(whichoutProbability) {
            if(possibleNext[i] > 0.0) {
                possibleNext[i] = 1.0;
            }
        }
        totalProbability *= possibleNext[i];
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
    for(uint8_t i = 0; i < 16; i++)
    {
        probability += m_possibleNext[i];
        if(probability >= (float)randVal) {
            return i;
        }
    }
    return 0;
}

}
