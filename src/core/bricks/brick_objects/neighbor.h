#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include <common.h>

#include <core/messaging/message_buffer/incoming_buffer.h>
#include <core/messaging/message_buffer/outgoing_buffer.h>

#include <core/bricks/brick_objects/brick_pos.h>

namespace KyoukoMind
{

//==================================================================================================

struct Neighbor
{
    uint8_t inUse = 0;

    BrickID targetBrickId = UNINIT_STATE_32;
    BrickPos targetBrickPos;
    uint8_t targetSide = 0;

    OutgoingBuffer outgoBuffer;
    IncomingBuffer incomBuffer;

} __attribute__((packed));

}

#endif // NEIGHBOR_H
