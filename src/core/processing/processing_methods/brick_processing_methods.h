/**
 *  @file    brick_processing_methods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef BRICK_PROCESSING_METHODS_H
#define BRICK_PROCESSING_METHODS_H

#include <common.h>

namespace KyoukoMind
{
struct Brick;
struct NetworkSegment;

void initCycle(Brick* brick);
void finishCycle(Brick* brick,
                 DataBuffer &monitoringMessage,
                 DataBuffer &clientMessage);
void finishSide(Brick* brick, const uint8_t side);
bool processReady(Brick* brick);
bool isReady(Brick* brick);

uint16_t processNodes(NetworkSegment &segment, Brick &brick);

void postLearning(NetworkSegment &segment, Brick &brick);
void memorizeSynapses(NetworkSegment &segment, Brick &brick);

void writeMonitoringOutput(Brick &brick, DataBuffer &buffer);
void writeClientOutput(Brick &brick, DataBuffer &buffer);

} // namespace KyoukoMind

#endif // BRICK_PROCESSING_METHODS_H
