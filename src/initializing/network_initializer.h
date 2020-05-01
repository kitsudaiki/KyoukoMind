/**
 *  @file    network_initializer.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef NETWORK_INITIALIZER_H
#define NETWORK_INITIALIZER_H

#include <common.h>
#include "init_meta_data.h"

namespace KyoukoMind
{

class BrickHandler;
struct NetworkSegment;

bool createNewNetwork(const std::string &fileContent);

void connectAllBricks(InitStructure &networkMetaStructure);

void addBricks(NetworkSegment &segment,
               InitStructure &networkMetaStructure);

uint32_t getNumberOfNodeBricks(InitStructure &networkMetaStructure);

std::pair<uint32_t, uint32_t> getNext(const uint32_t x,
                                      const uint32_t y,
                                      const uint8_t side);

uint32_t getDistantToNextNodeBrick(const uint32_t x,
                                   const uint32_t y,
                                   const uint8_t side);


} // namespace KyoukoMind

#endif // NETWORK_INITIALIZER_H
