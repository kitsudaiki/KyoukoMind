#include "nextchooser.h"

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
        calculatePossebilities(totalDistance);
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
 * @brief NextChooser::checkPossebilities
 * @return
 */
uint32_t NextChooser::checkPossebilities()
{
    uint32_t totalDistance = 0;
    // check types
    if(m_possibleNext[0].neighbor.neighborType == EMPTYCLUSTER) {
        m_possibleNext[0].good = false;
    } else { totalDistance+= m_possibleNext[0].neighbor.distantToNextNodeCluster; }
    if(m_possibleNext[1].neighbor.neighborType == EMPTYCLUSTER) {
        m_possibleNext[1].good = false;
    } else { totalDistance+= m_possibleNext[1].neighbor.distantToNextNodeCluster; }
    if(m_possibleNext[2].neighbor.neighborType == EMPTYCLUSTER) {
        m_possibleNext[2].good = false;
    } else { totalDistance+= m_possibleNext[2].neighbor.distantToNextNodeCluster; }

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
    if(m_possibleNext[0].good) {
        m_possibleNext[0].probability =
            100 - (uint8_t)(((float)m_possibleNext[0].neighbor.distantToNextNodeCluster
                            / (float)totalDistance) * 100.0);
        found = true;
    }
    if(m_possibleNext[1].good) {
        m_possibleNext[1].probability =
            100 - (uint8_t)(((float)m_possibleNext[1].neighbor.distantToNextNodeCluster
                            / (float)totalDistance) * 100.0);
        found = true;
    }
    if(m_possibleNext[2].good) {
        m_possibleNext[2].probability =
            100 - (uint8_t)(((float)m_possibleNext[2].neighbor.distantToNextNodeCluster
                            / (float)totalDistance) * 100.0);
        found = true;
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
    uint8_t probability = m_possibleNext[0].probability;
    if(m_possibleNext[0].good && probability <= randVal) {
        return m_possibleNext[0].neighbor.side;
    }
    probability += m_possibleNext[1].probability;
    if(m_possibleNext[1].good && probability <= randVal) {
        return m_possibleNext[1].neighbor.side;
    }
    probability += m_possibleNext[2].probability;
    if(m_possibleNext[2].good && probability <= randVal) {
        return m_possibleNext[2].neighbor.side;
    }
    return 0xFF;
}

}
