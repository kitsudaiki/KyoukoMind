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
    uint32_t sourceEdgeId = UNINIT_STATE_32;
    float weight = 0.0f;
    // total size: 12 Byte
};

//==================================================================================================

struct AxonTransfer
{
    uint32_t targetId = 0;
    float weight = 0.0f;
    // total size: 8 Byte
};

//==================================================================================================

struct UpdateTransfer
{
    uint32_t targetId = 0;
    float weightDiff = 0.0f;
    // total size: 8 Byte
};

//==================================================================================================

struct RandTransfer
{
    float randWeight[999];
    uint32_t randPos[1024];
};

//==================================================================================================

#endif // TRANSFER_OBJECTS_H
