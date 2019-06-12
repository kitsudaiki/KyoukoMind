/**
 *  @file    commonBrickMethods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef COMMONBRICKMETHODS_H
#define COMMONBRICKMETHODS_H

#include <common.h>

#include <core/messaging/messageMarker/incomingBuffer.h>
#include <core/messaging/messageMarker/outgoingBuffer.h>

namespace KyoukoMind
{

class MessageBlockBuffer;
class OutgoingMessageBuffer;
struct Brick;


bool addClientConnection(Brick *brick,
                         bool input,
                         bool output);

float getSummedValue(Brick *brick);

bool connectBricks(Brick* sourceBrick,
                   const uint8_t sourceSide,
                   Brick* targetBrick);

bool disconnectBricks(Brick* sourceBrick,
                      const uint8_t sourceSide,
                      Brick *targetBrick);

bool initNeighbor(Brick* brick,
                  const uint8_t sourceSide,
                  const uint32_t targetBrickId = UNINIT_STATE_32);

bool uninitNeighbor(Brick* brick,
                    const uint8_t side);

}

#endif // COMMONBRICKMETHODS_H
