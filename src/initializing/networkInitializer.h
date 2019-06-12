/**
 *  @file    networkInitializer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef NETWORKINITIALIZER_H
#define NETWORKINITIALIZER_H

#include <common.h>
#include "initMetaData.h"

namespace KyoukoMind
{

class BrickHandler;

bool createNewNetwork(const std::string fileContent);

bool connectAllBricks(InitStructure* networkMetaStructure);

void addBricks(const uint32_t nodeNumberPerBrick,
               std::vector<std::vector<InitMetaDataEntry> > *networkMetaStructure);

std::pair<uint32_t, uint32_t> getNext(const uint32_t x,
                                      const uint32_t y,
                                      const uint8_t side);

uint32_t getDistantToNextNodeBrick(const uint32_t x,
                                   const uint32_t y,
                                   const uint8_t side);


}

#endif // NETWORKINITIALIZER_H
