#include <core/methods/brick_initializing_methods.h>

#include <core/methods/neighbor_methods.h>
#include <core/processing/methods/brick_processing.h>
#include <core/methods/brick_item_methods.h>
#include <core/methods/data_connection_methods.h>

namespace KyoukoMind
{

//==================================================================================================

/**
 * initialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
initBrickNeighbor(Brick &sourceBrick,
                  const uint8_t sourceSide,
                  Brick* targetBrick,
                  Neighbor* targetNeighbor)
{

    // get and check neighbor
    Neighbor* neighbor = &sourceBrick.neighbors[sourceSide];
    assert(neighbor->inUse == 0);

    // init neighbor
    initNeighbor(*neighbor, targetBrick, targetNeighbor);

    return true;
}

//==================================================================================================

/**
 * uninitialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
uninitBrickNeighbor(Brick &sourceBrick,
                    const uint8_t side)
{
    // get and check neighbor
    Neighbor* neighbor = &sourceBrick.neighbors[side];
    assert(neighbor->inUse == 1);

    // uninit
    // TODO: issue #58
    neighbor->inUse = 0;

    return true;
}

//==================================================================================================

/**
 * connect two bricks by initialing the neighbors betweens the two bricks
 *
 * @return true, if successful, else false
 */
bool
connectBricks(Brick &sourceBrick,
              const uint8_t sourceSide,
              Brick &targetBrick)
{
    assert(sourceSide < 23);

    // get neighbor-pointers
    Neighbor* sourceNeighbor = &sourceBrick.neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick.neighbors[23-sourceSide];

    // check neighbors
    if(sourceNeighbor->inUse == 1
            || targetNeighbor->inUse == 1)
    {
        return false;
    }

    // init the new neighbors
    initBrickNeighbor(sourceBrick,
                      sourceSide,
                      &targetBrick,
                      targetNeighbor);
    initBrickNeighbor(targetBrick,
                      23 - sourceSide,
                      &sourceBrick,
                      sourceNeighbor);

    return true;
}

//==================================================================================================

/**
 * remove the connection between two neighbors
 *
 * @return true, if successful, else false
 */
bool
disconnectBricks(Brick &sourceBrick,
                 const uint8_t sourceSide,
                 Brick &targetBrick)
{
    assert(sourceSide < 23);

    // get neighbor-pointers
    Neighbor* sourceNeighbor = &sourceBrick.neighbors[sourceSide];
    Neighbor* targetNeighbor = &targetBrick.neighbors[23 - sourceSide];

    // check neighbors
    if(sourceNeighbor->inUse == 0
            || targetNeighbor->inUse == 0)
    {
        return false;
    }

    // add the new neighbor
    uninitBrickNeighbor(sourceBrick, sourceSide);
    uninitBrickNeighbor(targetBrick, 23 - sourceSide);

    return true;
}

//==================================================================================================

/**
 * initialize forward-edge-block
 *
 * @return true if success, else false
 */
bool
initEdgeSectionBlocks(Brick &brick,
                      const uint32_t numberOfEdgeSections)
{
    if(brick.edges.inUse != 0)
    {
        // TODO: log-output
        return false;
    }

    // init
    if(initDataBlocks(brick.edges,
                      numberOfEdgeSections,
                      sizeof(EdgeSection)) == false)
    {
        return false;
    }

    // fill array with empty forward-edge-sections
    EdgeSection* array = getEdgeBlock(brick);
    for(uint32_t i = 0; i < numberOfEdgeSections; i++)
    {
        // create new edge-section
        EdgeSection newSection;

        // connect all available sides
        for(uint8_t side = 0; side < 21; side++)
        {
            if(brick.neighbors[side].inUse != 0) {
                newSection.edges[side].available = 1;
            }
        }

        array[i] = newSection;
    }

    brick.edges.inUse = 1;

    return true;
}

//==================================================================================================

/**
 * @brief initRandValues
 * @param brick
 */
void
initRandValues(Brick &brick)
{
    brick.randWeight = new float[999];
    float compare = 0.0f;
    for(uint32_t i = 0; i < 999; i++)
    {
        if(i % 3 == 0) {
            compare = 0.0f;
        }

        float tempValue = static_cast<float>(rand()) / 0x7FFFFFFF;
        assert(tempValue <= 1.0f);
        if(tempValue + compare > 1.0f) {
            tempValue = 1.0f - compare;
        }
        compare += tempValue;
        brick.randWeight[i] = tempValue;
    }

    brick.randValue = new uint32_t[1024];
    for(uint32_t i = 0; i < 1024; i++)
    {
        brick.randValue[i] = static_cast<uint32_t>(rand());
    }
}

//==================================================================================================

}
