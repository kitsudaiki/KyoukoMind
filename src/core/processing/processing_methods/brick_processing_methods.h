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
#include <core/bricks/brick_objects/brick.h>

namespace Kitsune {
namespace Chan {
namespace Common {
struct TransferDataMessage;
}
}
}
using Kitsune::Chan::Common::TransferDataMessage;

namespace KyoukoMind
{

uint16_t processOutputNodes(Brick* brick);
uint16_t processNodes(Brick* brick, float *weightMap);

void postLearning(Brick* brick);
void memorizeSynapses(Brick* brick);

bool finishSide(Brick* brick,
                const uint8_t side);
void finishCycle(Brick* brick,
                 TransferDataMessage* monitoringMessage,
                 TransferDataMessage* clientMessage);

void writeStatus(Brick* brick, TransferDataMessage *message);
void writeOutput(Brick* brick, TransferDataMessage* message);

} // namespace KyoukoMind

#endif // BRICK_PROCESSING_METHODS_H
