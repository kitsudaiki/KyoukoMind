/**
 * @file        brick_initializer.cpp
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

#include "brick_initializer.h"

BrickInitializer::BrickInitializer()
{

}

BrickInitializer::~BrickInitializer()
{

}

bool BrickInitializer::initializeAxons(SynapseSegment &segment)
{
    uint32_t nodeCounter = 0;

    // calculate number of axons per brick
    for(uint32_t i = 0; i < segment.segmentMeta->numberOfNodeBricks; i++)
    {
        if(segment.nodeBricks[i].isOutputBrick == 1)
        {
            nodeCounter += segment.segmentMeta->numberOfNodesPerBrick;
            continue;
        }

        Brick* sourceBrick = &segment.nodeBricks[i];

        // iterate over all nodes of the brick and create an axon for each node
        for(uint32_t nodePos = 0; nodePos < segment.segmentMeta->numberOfNodesPerBrick; nodePos++)
        {
            Brick* axonBrick = getAxonBrick(segment, sourceBrick);
            assert(axonBrick->nodeBrickId <= 100);

            // calculate distance with pythagoras
            int32_t x = axonBrick->brickPos.x - sourceBrick->brickPos.x;
            int32_t y = axonBrick->brickPos.y - sourceBrick->brickPos.y;
            int32_t z = axonBrick->brickPos.z - sourceBrick->brickPos.z;
            x = x * x;
            y = y * y;
            z = z * z;
            const double dist = std::sqrt(x + y + z);

            // set source and target in related nodes and edges
            //edges[pos + nodePos].axonBrickId = axonBrick->brickId;
            segment.nodes[nodeCounter].nodeBrickId = sourceBrick->nodeBrickId;
            segment.nodes[nodeCounter].targetBrickDistance = static_cast<uint32_t>(dist);
            segment.nodes[nodeCounter].targetSectionId = nodeCounter;

            segment.synapseSections[nodeCounter].randomPos = rand() % 1024;

            // post-check
            assert(axonBrick->nodeBrickId != UNINIT_STATE_32);
            assert(sourceBrick->brickId != UNINIT_STATE_32);

            nodeCounter++;
        }
    }

    assert(nodeCounter == segment.segmentMeta->numberOfNodes);

    return true;
}
