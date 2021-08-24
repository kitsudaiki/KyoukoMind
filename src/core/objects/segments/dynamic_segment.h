#ifndef DYNAMIC_SEGMENTS_H
#define DYNAMIC_SEGMENTS_H

#include <common.h>

#include <core/objects/segments/abstract_segment.h>

class DynamicSegment : public AbstractSegment
{
public:
    DynamicSegment(const uint32_t numberOfBricks,
                   const uint32_t numberOfNodes,
                   const uint64_t numberOfSynapseSections);
    ~DynamicSegment();

    Brick* bricks = nullptr;
    uint32_t* brickOrder = nullptr;
    Node* nodes = nullptr;

    SynapseSection* synapseSections = nullptr;

    void addBricksToSegment(const JsonItem &metaBase);
    bool initTargetBrickList();

private:
    SegmentHeader createNewHeader(const uint32_t numberOfBricks,
                                  const uint32_t numberOfNodes,
                                  const uint64_t numberOfSynapseSections);
    void initSegmentPointer(const SegmentHeader &header);
    void allocateSegment(SegmentHeader &header);
    void initDefaultValues(const uint32_t numberOfBricks,
                           const uint32_t numberOfNodes);

    Brick createNewBrick(const JsonItem &brickDef, const uint32_t id);
    void connectBrick(Brick *sourceBrick, const uint8_t side);
    void connectAllBricks();
    Position getNeighborPos(Position sourcePos, const uint8_t side);
};

#endif // DYNAMIC_SEGMENTS_H
