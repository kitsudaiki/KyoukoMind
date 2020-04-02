/**
 *  @file    common_brick_methods.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef COMMON_BRICK_METHODS_H
#define COMMON_BRICK_METHODS_H

#include <common.h>

#include <core/messaging/message_buffer/incoming_buffer.h>
#include <core/messaging/message_buffer/outgoing_buffer.h>

namespace KyoukoMind
{

class MessageBlockBuffer;
class OutgoingMessageBuffer;
struct Brick;


bool addClientConnection(Brick &brick,
                         const bool isInput,
                         const bool isOutput);

float getSummedValue(Brick *brick);


bool connectBricks(Brick &sourceBrick,
                   const uint8_t sourceSide,
                   Brick &targetBrick);

bool disconnectBricks(Brick &sourceBrick,
                      const uint8_t sourceSide,
                      Brick &targetBrick);


bool initNeighbor(Brick &brick,
                  const uint8_t sourceSide,
                  const uint32_t targetBrickId = UNINIT_STATE_32);

bool uninitNeighbor(Brick &brick,
                    const uint8_t side);

} // namespace KyoukoMind

#endif // COMMON_BRICK_METHODS_H
