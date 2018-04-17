/**
 *  @file    nextChooser.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef NEXTCHOOSER_H
#define NEXTCHOOSER_H

#include <common.h>
#include <core/structs/clusterMeta.h>

namespace KyoukoMind
{

class NextChooser
{
public:
    struct PossibleNext
    {
        bool good = true;
        Neighbor neighbor;
        uint8_t probability = 33;
    };

    NextChooser();

    uint8_t getNextCluster(Neighbor* allNeighbors,
                           const uint8_t initialSite,
                           bool whichoutProbability = false);

private:
    PossibleNext m_possibleNext[0xF];
    uint8_t m_numberOfNext = 0;

    void init();

    void getPossibleNeighbors(Neighbor* allNeighbors,
                              const uint8_t initialSite);
    uint32_t checkPossebilities();
    bool calculatePossebilities(const uint32_t totalDistance);
    uint8_t chooseNeighbor();
};

}

#endif // NEXTCHOOSER_H
