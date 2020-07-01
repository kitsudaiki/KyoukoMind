/**
 *  @file    brick_cycle_methods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef BRICK_PROCESSING_METHODS_H
#define BRICK_PROCESSING_METHODS_H

#include <common.h>

namespace KyoukoMind
{
struct Brick;
struct NetworkSegment;

void initCycle(Brick* brick);
void finishCycle(Brick* brick);
void finishSide(Brick* brick, const uint8_t side);
bool processReady(Brick* brick);
bool isReady(Brick* brick);

void writeMonitoringOutput(Brick &brick,
                           DataBuffer &buffer);
void writeClientOutput(NetworkSegment &segment,
                       Brick &brick,
                       DataBuffer &buffer);

} // namespace KyoukoMind

#endif // BRICK_PROCESSING_METHODS_H
