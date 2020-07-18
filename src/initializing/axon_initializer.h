/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef AXON_INITIALIZER_H
#define AXON_INITIALIZER_H

#include <common.h>
#include "init_meta_data.h"
#include <core/object_handling/brick.h>

namespace KyoukoMind
{
class NextChooser;

struct Neighbor;
class Segment;

struct NewAxon {
    uint32_t x = 0;
    uint32_t y = 0;
};


bool createAxons(Segment &segment,
                 const std::vector<std::vector<InitMetaDataEntry> > &networkMetaStructure);

NewAxon getNextAxonPathStep(const uint32_t x,
                            const uint32_t y,
                            const uint8_t inputSide,
                            uint32_t &currentStep,
                            const std::vector<std::vector<InitMetaDataEntry>> &networkMetaStructure);

uint8_t chooseNextSide(const uint8_t initialSide,
                       uint32_t* neighbors);

} // namespace KyoukoMind

#endif // AXON_INITIALIZER_H
