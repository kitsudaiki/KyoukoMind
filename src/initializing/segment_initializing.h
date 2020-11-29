#ifndef SEGMENT_INITIALIZING_H
#define SEGMENT_INITIALIZING_H

#include <common.h>

class Segment;

bool initBricks(Segment &segment,
                const uint32_t numberOfBricks);
bool initGlobalValues(Segment &segment);
bool initNodeBlocks(Segment &segment,
                    const uint32_t &numberOfNodes);
bool initRandomValues(Segment &segment);
bool initEdgeSectionBlocks(Segment &segment,
                           const uint32_t numberOfEdgeSections);
bool initSynapseSectionBlocks(Segment &segment,
                              const uint32_t numberOfSynapseSections);
bool initTransferBlocks(Segment &segment,
                        const uint32_t totalNumberOfAxons,
                        const uint64_t maxNumberOySynapseSections);

#endif // SEGMENT_INITIALIZING_H
