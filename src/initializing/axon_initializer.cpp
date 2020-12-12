/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "axon_initializer.h"
#include <kyouko_root.h>

#include <core/objects/segment.h>
#include <core/objects/global_values.h>

#include <core/objects/node.h>

/**
 * @brief createAxons
 * @return
 */
bool
createAxons(Segment &segment,
            const std::vector<std::vector<InitMetaDataEntry>> &networkMetaStructure)
{
    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

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

            if(brick->nodeBrickId != UNINIT_STATE_32)
            {
                // get node-brick
                EdgeSection* edges = getBuffer<EdgeSection>(segment.edges);
                Node* nodes = getBuffer<Node>(segment.nodes);

                const uint32_t pos = brick->nodeBrickId * globalValues->numberOfNodesPerBrick;

                // iterate over all nodes of the brick and create an axon for each node
                for(uint32_t i = 0; i < globalValues->numberOfNodesPerBrick; i++)
                {
                    // create new axon
                    uint32_t lenght = 0;
                    const NewAxon target = getNextAxonPathStep(x,
                                                               y,
                                                               0,
                                                               lenght,
                                                               networkMetaStructure);

                    Brick* targetBrick = networkMetaStructure[target.x][target.y].brick;
                    edges[pos + i].targetBrickId = targetBrick->brickId;
                    nodes[pos + i].brickId = brick->brickId;
                    nodes[pos + i].targetBrickDistance = lenght;
                }
            }
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
                    uint32_t &currentStep,
                    const std::vector<std::vector<InitMetaDataEntry>> &networkMetaStructure)
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
        result.x = x;
        result.y = y;
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
        result.x = x;
        result.y = y;
        return result;
    }

    // get the neighbor of the choosen side
    const uint32_t choosenOne = networkMetaStructure[x][y].brick->neighbors[nextSite];
    Brick* targetBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[choosenOne];

    // make next iteration
    currentStep++;
    return getNextAxonPathStep(targetBrick->brickPos.x,
                               targetBrick->brickPos.y,
                               23 - nextSite,
                               currentStep,
                               networkMetaStructure);
}

/**
 * @brief AxonInitializer::chooseNextSide
 * @param neighbors
 * @return
 */
uint8_t
chooseNextSide(const uint8_t initialSide,
               uint32_t* neighbors)
{
    const std::vector<uint8_t> sideOrder = {9,10,11,14,13,12};
    std::vector<uint8_t> availableSides;

    for(uint8_t i = 0; i < sideOrder.size(); i++)
    {
        if(neighbors[sideOrder[i]] != UNINIT_STATE_32
                && sideOrder[i] != initialSide)
        {
            availableSides.push_back(sideOrder[i]);
        }
    }

    if(availableSides.size() != 0) {
        return availableSides[static_cast<uint32_t>(rand()) % availableSides.size()];
    }

    return 0xFF;
}
