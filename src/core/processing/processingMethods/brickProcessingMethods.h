#ifndef BRICKPROCESSINGMETHODS_H
#define BRICKPROCESSINGMETHODS_H

#include <common.h>
#include <core/bricks/brickObjects/brick.h>

namespace Kitsune {
namespace Chan {
namespace Communication {
struct TransferDataMessage;
}
}
}
using Kitsune::Chan::Communication::TransferDataMessage;

namespace KyoukoMind
{

uint16_t processOutputNodes(Brick* brick);
uint16_t processNodes(Brick* brick, float *weightMap);

void postLearning(Brick* brick);
void memorizeEdges(Brick* brick);

bool finishSide(Brick* brick,
                const uint8_t side);
void finishCycle(Brick* brick,
                 TransferDataMessage* monitoringMessage,
                 TransferDataMessage* clientMessage);

void writeStatus(Brick* brick, TransferDataMessage *message);
void writeOutput(Brick* brick, TransferDataMessage* message);

}

#endif // BRICKPROCESSINGMETHODS_H
