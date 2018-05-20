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
                                       float *m_possibleNext)
{
    switch((int)initialSite)
    {
    case 0:
        m_possibleNext[2] = 1.0;
        m_possibleNext[3] = 1.0;
        m_possibleNext[4] = 1.0;
        m_possibleNext[11] = 1.0;
        m_possibleNext[12] = 1.0;
        m_possibleNext[13] = 1.0;
        break;
    case 2:
        m_possibleNext[4] = allNeighbors[4].distantToNextNodeCluster
                + allNeighbors[4].activeNodesInNextNodeCluster;
        m_possibleNext[12] = allNeighbors[12].distantToNextNodeCluster
                + allNeighbors[12].activeNodesInNextNodeCluster;
        m_possibleNext[13] = allNeighbors[13].distantToNextNodeCluster
                + allNeighbors[13].activeNodesInNextNodeCluster;
        break;
    case 3:
        m_possibleNext[11] = allNeighbors[11].distantToNextNodeCluster
                + allNeighbors[11].activeNodesInNextNodeCluster;
        m_possibleNext[12] = allNeighbors[12].distantToNextNodeCluster
                + allNeighbors[12].activeNodesInNextNodeCluster;
        m_possibleNext[13] = allNeighbors[13].distantToNextNodeCluster
                + allNeighbors[13].activeNodesInNextNodeCluster;
        break;
    case 4:
        m_possibleNext[2] = allNeighbors[2].distantToNextNodeCluster
                + allNeighbors[2].activeNodesInNextNodeCluster;
        m_possibleNext[11] = allNeighbors[11].distantToNextNodeCluster
                + allNeighbors[11].activeNodesInNextNodeCluster;
        m_possibleNext[12] = allNeighbors[12].distantToNextNodeCluster
                + allNeighbors[12].activeNodesInNextNodeCluster;
        break;
    case 11:
        m_possibleNext[3] = allNeighbors[3].distantToNextNodeCluster
                + allNeighbors[3].activeNodesInNextNodeCluster;
        m_possibleNext[4] = allNeighbors[4].distantToNextNodeCluster
                + allNeighbors[4].activeNodesInNextNodeCluster;
        m_possibleNext[13] = allNeighbors[13].distantToNextNodeCluster
                + allNeighbors[13].activeNodesInNextNodeCluster;
        break;
    case 12:
        m_possibleNext[2] = allNeighbors[2].distantToNextNodeCluster
                + allNeighbors[2].activeNodesInNextNodeCluster;
        m_possibleNext[3] = allNeighbors[3].distantToNextNodeCluster
                + allNeighbors[3].activeNodesInNextNodeCluster;
        m_possibleNext[4] = allNeighbors[4].distantToNextNodeCluster
                + allNeighbors[4].activeNodesInNextNodeCluster;
        break;
    case 13:
        m_possibleNext[11] = allNeighbors[11].distantToNextNodeCluster
                + allNeighbors[11].activeNodesInNextNodeCluster;
        m_possibleNext[2] = allNeighbors[2].distantToNextNodeCluster
                + allNeighbors[2].activeNodesInNextNodeCluster;
        m_possibleNext[3] = allNeighbors[3].distantToNextNodeCluster
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
                                          float *m_possibleNext)
{
    float totalProbability = 0.0;

    for(uint8_t i = 0; i < 16; i++)
    {
        if(whichoutProbability) {
            if(m_possibleNext[i] > 0.0) {
                m_possibleNext[i] = 1.0;
            }
        }
        totalProbability *= m_possibleNext[i];
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
        if(probability >= randVal) {
            return i;
        }
    }
    return 0;
}

}
