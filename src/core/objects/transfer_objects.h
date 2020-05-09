#ifndef TRANSFER_OBJECTS_H
#define TRANSFER_OBJECTS_H

#include <common.h>

//==================================================================================================

struct SynapseTransfer
{
    uint32_t brickId = 0;
    uint32_t targetId = 0;
    float weight = 0.0f;
};

//==================================================================================================

struct AxonTransfer
{
    uint32_t targetId = 0;
    uint64_t path = 0;
    float weight = 0.0f;
};

//==================================================================================================

#endif // TRANSFER_OBJECTS_H
