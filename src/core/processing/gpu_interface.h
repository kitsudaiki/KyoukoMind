#ifndef GPU_INTERFACE_H
#define GPU_INTERFACE_H

#include <common.h>

#include <core/objects/container_definitions.h>
#include <core/objects/synapses.h>
#include <core/objects/network_segment.h>

namespace KyoukoMind
{

bool initializeGpu(NetworkSegment &segment,
                   const uint32_t numberOfBricks);
bool copyEdgesToGpu(NetworkSegment &segment);
bool runOnGpu(NetworkSegment &segment);
bool copyAxonsFromGpu(NetworkSegment &segment);
bool closeDevice(NetworkSegment &segment);

}

#endif // GPU_INTERFACE_H
