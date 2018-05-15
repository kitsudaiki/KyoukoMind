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
    init();
}

/**
 * @brief NextChooser::init
 */
void NextChooser::init()
{
    PossibleNext temp;
    for(int i = 0; i < 0xF; i++) {
        m_possibleNext[i] = temp;
    }
}

/**
 * @brief NextChooser::getNextCluster
 * @param allNeighbors
 * @param initialSite
 * @param whichoutProbability
 * @return
 */
uint8_t NextChooser::getNextCluster(Neighbor *allNeighbors,
                                    const uint8_t initialSite,
                                    bool whichoutProbability)
{
    if(initialSite > 0xF || allNeighbors == nullptr) {
        return 0xFF;
    }

    getPossibleNeighbors(allNeighbors, initialSite);

    float totalProbability = calculatePossebilities(whichoutProbability);
    if(totalProbability == 0.0) {
        return initialSite;
    }
    return chooseNeighbor(totalProbability);
}


/**
 * @brief NextChooser::getPossibleNeighbors
 * @param allNeighbors
 * @param initialSite
 */
void NextChooser::getPossibleNeighbors(Neighbor* allNeighbors,
                                       const uint8_t initialSite)
{
    switch((int)initialSite)
    {
    case 0:
        m_possibleNext[0].neighbor = allNeighbors[2];
        m_possibleNext[1].neighbor = allNeighbors[3];
        m_possibleNext[2].neighbor = allNeighbors[4];
        m_possibleNext[3].neighbor = allNeighbors[11];
        m_possibleNext[4].neighbor = allNeighbors[12];
        m_possibleNext[5].neighbor = allNeighbors[13];
        m_numberOfNext = 6;
        break;
    case 2:
        m_possibleNext[0].neighbor = allNeighbors[4];
        m_possibleNext[1].neighbor = allNeighbors[12];
        m_possibleNext[2].neighbor = allNeighbors[13];
        m_numberOfNext = 3;
        break;
    case 3:
        m_possibleNext[0].neighbor = allNeighbors[11];
        m_possibleNext[1].neighbor = allNeighbors[12];
        m_possibleNext[2].neighbor = allNeighbors[13];
        m_numberOfNext = 3;
        break;
    case 4:
        m_possibleNext[0].neighbor = allNeighbors[2];
        m_possibleNext[1].neighbor = allNeighbors[11];
        m_possibleNext[2].neighbor = allNeighbors[12];
        m_numberOfNext = 3;
        break;
    case 11:
        m_possibleNext[0].neighbor = allNeighbors[3];
        m_possibleNext[1].neighbor = allNeighbors[4];
        m_possibleNext[2].neighbor = allNeighbors[13];
        m_numberOfNext = 3;
        break;
    case 12:
        m_possibleNext[0].neighbor = allNeighbors[2];
        m_possibleNext[1].neighbor = allNeighbors[3];
        m_possibleNext[2].neighbor = allNeighbors[4];
        m_numberOfNext = 3;
        break;
    case 13:
        m_possibleNext[0].neighbor = allNeighbors[11];
        m_possibleNext[1].neighbor = allNeighbors[2];
        m_possibleNext[2].neighbor = allNeighbors[3];
        m_numberOfNext = 3;
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
float NextChooser::calculatePossebilities(const bool whichoutProbability)
{
    float totalProbability = 0.0;

    for(uint8_t i = 0; i < m_numberOfNext; i++)
    {
        if(whichoutProbability)
        {
            if(m_possibleNext[i].neighbor.neighborType == EMPTY_CLUSTER) {
                m_possibleNext[i].probability = (float)MAX_DISTANCE / (m_numberOfNext);
            } else {
                m_possibleNext[i].probability = 0;
            }
        }
        else
        {
            m_possibleNext[i].probability =
                (MAX_DISTANCE - m_possibleNext[i].neighbor.distantToNextNodeCluster) *
                    m_possibleNext[i].neighbor.activeNodesInNextNodeCluster;
        }

        totalProbability += m_possibleNext[i].probability;
    }
    return totalProbability;
}

/**
 * @brief NextChooser::chooseNeighbor
 * @return
 */
uint8_t NextChooser::chooseNeighbor(const float totalProbability)
{
    // choose a side
    uint32_t randVal = rand() % (uint32_t)totalProbability;
    uint8_t probability = 0;
    for(uint8_t i = 0; i < m_numberOfNext; i++)
    {
        probability += m_possibleNext[i].probability;
        if(probability >= randVal) {
            return 15 - m_possibleNext[i].neighbor.targetSide;
        }
    }
    return 15 - m_possibleNext[0].neighbor.targetSide;
}

}
