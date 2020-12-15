/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef TRANSFER_OBJECTS_H
#define TRANSFER_OBJECTS_H

#include <common.h>

//==================================================================================================

struct SynapseTransfer
{
    uint32_t brickId = UNINIT_STATE_32;
    uint32_t nodeBrickId = UNINIT_STATE_32;
    uint32_t synapseSectionId = UNINIT_STATE_32;
    uint32_t sourceEdgeId = UNINIT_STATE_32;
    uint8_t positionInEdge = UNINIT_STATE_8;
    uint8_t isNew = 0;
    uint8_t padding[2];
    float weight = 0.0f;
    // total size: 24 Byte
};

//==================================================================================================

struct AxonTransfer
{
    float weight = 0.0f;
    uint32_t brickId = UNINIT_STATE_32;
    // total size: 8 Byte
};

//==================================================================================================

struct UpdateTransfer
{
    uint32_t targetId = UNINIT_STATE_32;
    uint8_t positionInEdge = UNINIT_STATE_8;
    uint8_t deleteEdge = 0;
    uint8_t padding[2];
    float newWeight = 0.0f;
    // total size: 12 Byte
};

//==================================================================================================

#endif // TRANSFER_OBJECTS_H
