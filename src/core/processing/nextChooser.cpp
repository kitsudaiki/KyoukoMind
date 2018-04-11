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

    uint32_t totalDistance = checkPossebilities();

    if(!whichoutProbability) {
        if(!calculatePossebilities(totalDistance)) {
            return 0xFF;
        }
    }

    return chooseNeighbor();
}


/**
 * @brief NextChooser::getPossibleNeighbors
 * @param allNeighbors
 * @param initialSite
 */
void NextChooser::getPossibleNeighbors(Neighbor* allNeighbors,
                                       const uint8_t initialSite)
{
    switch((int)initialSite) {
    case 1:
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
    case 14:
        m_possibleNext[0].neighbor = allNeighbors[2];
        m_possibleNext[1].neighbor = allNeighbors[3];
        m_possibleNext[2].neighbor = allNeighbors[4];
        m_possibleNext[3].neighbor = allNeighbors[11];
        m_possibleNext[4].neighbor = allNeighbors[12];
        m_possibleNext[5].neighbor = allNeighbors[13];
        m_numberOfNext = 6;
        break;
    default:
        break;
    }
}

/**
 * @brief NextChooser::checkPossebilities
 * @return
 */
uint32_t NextChooser::checkPossebilities()
{
    uint32_t totalDistance = 0;
    // check types
    for(uint8_t i = 0; i < m_numberOfNext; i++)
    {
        if(m_possibleNext[i].neighbor.neighborType == EMPTYCLUSTER) {
            m_possibleNext[i].good = false;
        } else {
            m_possibleNext[i].good = true;
            totalDistance += m_possibleNext[i].neighbor.distantToNextNodeCluster;
        }
    }
    return totalDistance;
}

/**
 * @brief NextChooser::calculatePossebilities
 * @param totalDistance
 * @return
 */
bool NextChooser::calculatePossebilities(const uint32_t totalDistance)
{
    bool found = false;

    for(uint8_t i = 0; i < m_numberOfNext; i++)
    {
        if(m_possibleNext[i].good) {
            m_possibleNext[i].probability =
                100 - (uint8_t)(((float)m_possibleNext[i].neighbor.distantToNextNodeCluster
                                / (float)totalDistance) * 100.0);
            found = true;
        }
    }
    return found;
}

/**
 * @brief NextChooser::chooseNeighbor
 * @return
 */
uint8_t NextChooser::chooseNeighbor()
{
    // choose a side
    int randVal = rand() % 100;
    uint8_t probability = 0;
    for(uint8_t i = 0; i < m_numberOfNext; i++)
    {
        if(m_possibleNext[i].good) {
            probability += m_possibleNext[i].probability;
            if(probability >= randVal) {
                return m_possibleNext[i].neighbor.side;
            }
        }
    }
    return 0xFF;
}

}
