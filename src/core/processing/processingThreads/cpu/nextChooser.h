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
    NextChooser();

    uint8_t getNextCluster(Neighbor* allNeighbors,
                           const uint8_t initialSite,
                           bool whichoutProbability = false);

private:
    void getPossibleNeighbors(Neighbor* allNeighbors,
                              const uint8_t initialSite,
                              float *m_possibleNext);
    float calculatePossebilities(bool whichoutProbability,
                                 float *m_possibleNext);
    uint8_t chooseNeighbor(const float totalProbability,
                           float *m_possibleNext);
};

}

#endif // NEXTCHOOSER_H
