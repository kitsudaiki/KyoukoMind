/**
 * @file        axon_initializer.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "axon_initializer.h"
#include <kyouko_root.h>

#include <core/objects/segment.h>
#include <core/objects/global_values.h>

#include <core/objects/node.h>

/**
 * @brief create new axons
 *
 * @param segment segment with the basic data
 * @param networkMetaStructure reference to resulting meta-structure
 *
 * @return true, if successfull, else false
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
 * @brief make iterative step to create an axon-path
 *
 * @param x current x-position
 * @param y current y-position
 * @param inputSide current incoming side
 * @param currentStep current step-count (max 8 steps)
 * @param networkMetaStructure reference to resulting meta-structure
 *
 * @return new axon object
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
 * @brief choose next side, which also has a connected neighbor
 *
 * @param initialSide incoming side
 * @param neighbors list of neighbors
 *
 * @return choosed side, 0xFF if no available side was found
 */
uint8_t
chooseNextSide(const uint8_t initialSide,
               uint32_t* neighbors)
{
    const std::vector<uint8_t> sideOrder = {9,10,11,14,13,12};
    std::vector<uint8_t> availableSides;

    // collect available sides
    for(uint8_t i = 0; i < sideOrder.size(); i++)
    {
        if(neighbors[sideOrder[i]] != UNINIT_STATE_32
                && sideOrder[i] != initialSide)
        {
            availableSides.push_back(sideOrder[i]);
        }
    }

    // choose one of the available side
    if(availableSides.size() != 0) {
        return availableSides[static_cast<uint32_t>(rand()) % availableSides.size()];
    }

    return 0xFF;
}
