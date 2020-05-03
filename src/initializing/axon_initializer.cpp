/**
 *  @file    axon_initializer.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#include "axon_initializer.h"
#include <root_object.h>

#include <core/objects/brick.h>
#include <core/methods/brick_item_methods.h>
#include <core/methods/brick_initializing_methods.h>
#include <core/methods/network_segment_methods.h>

namespace KyoukoMind
{

/**
 * @brief createAxons
 * @return
 */
bool
createAxons(NetworkSegment &segment,
            std::vector<std::vector<InitMetaDataEntry>> &networkMetaStructure)
{
    // calculate number of axons per brick
    for(uint32_t x = 0; x < networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < networkMetaStructure[x].size(); y++)
        {
            // check brick-type
            Brick* brick = networkMetaStructure[x][y].brick;
            if(brick == nullptr) {
                continue;
            }

            if(networkMetaStructure[x][y].type == NODE_BRICK)
            {
                // get node-brick
                uint32_t nodeNumberPerBrick = NUMBER_OF_NODES_PER_BRICK;
                Node* nodes = &getNodeBlock(segment)[brick->nodePos];

                // iterate over all nodes of the brick and create an axon for each node
                for(uint16_t nodeNumber = 0; nodeNumber < nodeNumberPerBrick; nodeNumber++)
                {
                    // create new axon
                    const NewAxon axon = getNextAxonPathStep(x, y, 0, 8, 1, networkMetaStructure);
                    InitMetaDataEntry entry = networkMetaStructure[axon.targetX][axon.targetY];
                    const uint32_t axonId = entry.numberOfAxons;

                    // update values of the brick and the node
                    networkMetaStructure[axon.targetX][axon.targetY].numberOfAxons++;
                    nodes[nodeNumber].targetBrickPath = axon.targetPath;
                    nodes[nodeNumber].targetAxonId = axonId;
                }
            }
        }
    }

    // add the calculated number of axons to all bricks
    for(uint32_t x = 0; x < networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < networkMetaStructure[x].size(); y++)
        {
            Brick* brick = networkMetaStructure[x][y].brick;
            if(brick == nullptr) {
                continue;
            }

            // add the axon-number to the specific brick
            if(networkMetaStructure[x][y].numberOfAxons == 0) {
                networkMetaStructure[x][y].numberOfAxons = 1;
            }

            initEdgeSectionBlocks(*brick, networkMetaStructure[x][y].numberOfAxons);
        }
    }

    return true;
}


/**
 * @brief AxonInitializer::getNextAxonPathStep
 * @param x
 * @param y
 * @param inputSide
 * @param currentPath
 * @param currentStep
 * @return
 */
NewAxon
getNextAxonPathStep(const uint32_t x,
                    const uint32_t y,
                    const uint8_t inputSide,
                    const uint64_t currentPath,
                    const uint32_t currentStep,
                    std::vector<std::vector<InitMetaDataEntry>> &networkMetaStructure)
{
    // check if go to next
    bool goToNext = false;
    if(rand() % 100 < POSSIBLE_NEXT_AXON_STEP) {
         goToNext = true;
    }

    // at least one axon-step
    if(inputSide == 0) {
        goToNext = true;
    }

    // return the current values if no next or path long enough
    if(goToNext == false
            || currentStep == 8)
    {
        NewAxon result;
        result.targetX = x;
        result.targetY = y;
        result.targetPath = currentPath;
        return result;
    }

    // choose the next brick
    uint8_t nextSite = 0xFF;
    if(networkMetaStructure[x][y].type != EMPTY_BRICK) {
        nextSite = chooseNextSide(inputSide, networkMetaStructure[x][y].brick->neighbors);
    }

    if(nextSite == 0xFF)
    {
        NewAxon result;
        result.targetX = x;
        result.targetY = y;
        result.targetPath = currentPath;
        return result;
    }

    // get the neighbor of the choosen side
    Neighbor* choosenOne = &networkMetaStructure[x][y].brick->neighbors[nextSite];

    // update path
    const uint64_t newPath = currentPath + ((uint64_t)nextSite << (currentStep * 5));

    // make next iteration
    return getNextAxonPathStep(choosenOne->targetBrickPos.x,
                               choosenOne->targetBrickPos.y,
                               23 - nextSite,
                               newPath,
                               currentStep + 1,
                               networkMetaStructure);
}

/**
 * @brief AxonInitializer::chooseNextSide
 * @param neighbors
 * @return
 */
uint8_t
chooseNextSide(const uint8_t initialSide, Neighbor* neighbors)
{
    const std::vector<uint8_t> sideOrder = {9,10,11,14,13,12};
    std::vector<uint8_t> availableSides;

    for(uint8_t i = 0; i < sideOrder.size(); i++)
    {
        if(neighbors[sideOrder[i]].targetBrick != nullptr
                && sideOrder[i] != initialSide)
        {
            availableSides.push_back(sideOrder[i]);
        }
    }

    if(availableSides.size() != 0) {
        return availableSides[(uint32_t)rand() % availableSides.size()];
    }

    return 0xFF;
}

} // namespace KyoukoMind
