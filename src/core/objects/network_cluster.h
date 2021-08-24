#ifndef NETWORKCLUSTER_H
#define NETWORKCLUSTER_H

#include <common.h>

class InputSegment;
class OutputSegment;
class AbstractSegment;

struct NetworkMetaData
{
    float lerningValue = 0.0f;
    uint32_t cycleTime = 1000000;

    uint8_t padding[247];
};

struct InitSettings
{
    uint16_t nodesPerBrick = 0;
    uint16_t maxBrickDistance = 0;
    float nodeLowerBorder = 0.0f;
    float nodeUpperBorder = 0.0f;
    uint32_t layer = 0;
    uint64_t maxSynapseSections = 0;

    uint8_t padding[232];
};

class NetworkCluster
{
public:
    NetworkCluster();

    NetworkMetaData networkMetaData;
    InitSettings initMetaData;

    std::vector<InputSegment*> inputSegments;
    std::vector<OutputSegment*> outputSegments;
    std::vector<AbstractSegment*> allSegments;
    std::deque<AbstractSegment*> segmentQueue;
};

#endif // NETWORKCLUSTER_H
