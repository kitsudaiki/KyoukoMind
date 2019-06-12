/**
 *  @file    axonInitializer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef AXONINITIALIZER_H
#define AXONINITIALIZER_H

#include <common.h>
#include "initMetaData.h"

namespace KyoukoMind
{
class NextChooser;
struct Neighbor;

struct NewAxon {
    uint32_t targetX = 0;
    uint32_t targetY = 0;
    uint64_t targetPath = 0;
};


bool createAxons(std::vector<std::vector<InitMetaDataEntry> > *networkMetaStructure);

NewAxon getNextAxonPathStep(const uint32_t x,
                            const uint32_t y,
                            const uint8_t inputSide,
                            const uint64_t currentPath,
                            const uint32_t currentStep,
                            std::vector<std::vector<InitMetaDataEntry> > *networkMetaStructure);

uint8_t chooseNextSide(const uint8_t initialSide, Neighbor* neighbors);

}

#endif // AXONINITIALIZER_H
