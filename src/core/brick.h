#ifndef BRICK_H
#define BRICK_H

#include <common.h>
#include <kyouko_root.h>

#include <core/processing/objects/edges.h>
#include <core/objects/data_connection.h>

namespace KyoukoMind
{

class Brick
{

public:

    //----------------------------------------------------------------------------------------------
    struct BrickPos
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
        uint32_t w = 0;

    } __attribute__((packed));
    //----------------------------------------------------------------------------------------------
    struct Neighbor
    {
        uint8_t inUse = 0;

        Brick* targetBrick = nullptr;
        Neighbor* targetNeighbor = nullptr;
        BrickPos targetBrickPos;

        std::queue<StackBuffer*> bufferQueue;
        StackBuffer* outgoingBuffer = nullptr;
        StackBuffer* currentBuffer = nullptr;

    } __attribute__((packed));
    //----------------------------------------------------------------------------------------------

    // common
    uint32_t brickId = UNINIT_STATE_32;
    uint32_t nodeBrickId = UNINIT_STATE_32;

    BrickPos brickPos;
    uint8_t inQueue = 0;
    uint8_t isOutputBrick = 0;
    uint8_t isInputBrick = 0;
    uint8_t isNodeBrick = 0;

    // random values
    float* randWeight = nullptr;
    uint32_t randWeightPos = 0;
    uint32_t* randValue = nullptr;
    uint32_t randValuePos = 0;

    // 0 - 21: neighbor-bricks
    // 22: the current brick
    Neighbor neighbors[23];
    std::atomic_flag lock = ATOMIC_FLAG_INIT;

    // data
    DataConnection edges;
    int32_t nodePos = -1;

    // learning metadata
    float learningOverride = 0.5;
    GlobalValues globalValues;

    //----------------------------------------------------------------------------------------------

    Brick(const uint32_t &brickId,
          const uint32_t x,
          const uint32_t y);
    ~Brick();

    void initCycle();
    void finishCycle();
    void finishSide(const uint8_t side);
    bool processReady();
    bool isReady();

    float getSummedValue(NetworkSegment &segment);

    bool connectBricks(const uint8_t sourceSide,
                       Brick &targetBrick);
    bool disconnectBricks(const uint8_t sourceSide,
                          Brick &targetBrick);

    bool initEdgeSectionBlocks(const uint32_t numberOfEdgeSections);
    uint64_t addEmptyEdgeSection(const uint8_t sourceSide,
                                 const uint32_t sourceId);

    void writeMonitoringOutput(DataBuffer &buffer);
    void writeClientOutput(NetworkSegment &segment,
                           DataBuffer &buffer);

private:
    void initRandValues();

    bool initBrickNeighbor(const uint8_t sourceSide,
                           Brick* targetBrick,
                           Neighbor* targetNeighbor);

    void initNeighbor(Neighbor &neighbor,
                      Brick* targetBrick,
                      Neighbor* targetNeighbor);
    bool uninitBrickNeighbor(const uint8_t side);

    void sendNeighborBuffer(Neighbor &sourceNeighbor,
                            Neighbor &targetNeighbor);
    void switchNeighborBuffer(Neighbor &neighbor);
};

inline EdgeSection* getEdgeBlock(Brick &brick)
{
    return static_cast<EdgeSection*>(brick.edges.buffer.data);
}

}

#endif // BRICK_H
