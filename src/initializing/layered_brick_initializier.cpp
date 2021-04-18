/**
 * @file        layered_brick_initializier.cpp
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

#include "layered_brick_initializier.h"

LayeredBrickInitializier::LayeredBrickInitializier()
    : BrickInitializer()
{

}

/**
 * @brief LayeredBrickInitializier::getAxonBrick
 * @param segment
 * @param sourceBrick
 * @return
 */
Brick*
LayeredBrickInitializier::getAxonBrick(SynapseSegment &segment, Brick* sourceBrick)
{
    const uint32_t sourceLayerId = sourceBrick->layerId;
    const uint32_t nextPos = static_cast<uint32_t>(rand())
                             % segment.layer[sourceLayerId + 1].size();
    return segment.layer[sourceLayerId + 1][nextPos];
}

/**
 * @brief LayeredBrickInitializier::initTargetBrickList
 * @param segment
 * @return
 */
bool
LayeredBrickInitializier::initTargetBrickList(SynapseSegment &segment,
                                              Kitsunemimi::Ai::InitMeataData* initMetaData)
{
    for(uint32_t i = 0; i < segment.layer.size() - 1; i++)
    {
        for(uint32_t j = 0; j < segment.layer[i].size(); j++)
        {
            Brick* baseBrick = segment.layer[i][j];

            for(uint32_t k = 0; k < 1000; k++)
            {
                const uint32_t nextPos = static_cast<uint32_t>(rand())
                                         % segment.layer[i + 1].size();
                Brick* targetBrick = segment.layer[i + 1][nextPos];
                baseBrick->possibleTargetNodeBrickIds[k] = targetBrick->nodeBrickId;
            }
        }
    }

    return true;
}

