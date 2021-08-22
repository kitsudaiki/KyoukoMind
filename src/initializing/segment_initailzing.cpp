/**
 * @file        segment_initailzing.cpp
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

#include "segment_initailzing.h"



/**
 * @brief initializeNodes
 * @param segment
 * @param initMetaData
 * @return
 */
bool
initializeNodes(Segment &segment,
                InitSettings* initMetaData)
{
    const uint32_t numberOfNodes = segment.segmentHeader->nodes.count;
    const float range = initMetaData->nodeUpperBorder - initMetaData->nodeLowerBorder;

    for(uint32_t i = 0; i < numberOfNodes; i++)
    {
        segment.nodes[i].border = fmod(static_cast<float>(rand()), range);
        segment.nodes[i].border += initMetaData->nodeLowerBorder;
    }

    return true;
}

Brick
createNewBrick(const JsonItem &brickDef, const uint32_t id)
{
    Brick newBrick;

    // copy metadata
    newBrick.brickId = id;
    newBrick.nodeBrickId = id;
    if(brickDef.contains("type"))
    {
        if(brickDef.get("type").getString() == "output") {
            newBrick.isOutputBrick = true;
        }
        if(brickDef.get("type").getString() == "input") {
            newBrick.isInputBrick = true;
        }
    }

    // copy position
    newBrick.brickPos.x = brickDef.get("x").getInt();
    newBrick.brickPos.y = brickDef.get("y").getInt();
    newBrick.brickPos.z = brickDef.get("z").getInt();

    // set other values
    newBrick.numberOfNodes = brickDef.get("number_of_nodes").getInt();

    return newBrick;
}

/**
 * @brief addBricksToSegment
 * @param segment
 * @param initMetaData
 * @param metaBase
 */
void
addBricksToSegment(Segment &segment,
                   InitSettings* initMetaData,
                   const JsonItem &metaBase)
{
    uint32_t nodeBrickIdCounter = 0;
    uint32_t inputCounter = 0;
    uint32_t nodePosCounter = 0;
    JsonItem bricks = metaBase.get("bricks");

    for(uint32_t i = 0; i < bricks.size(); i++)
    {
        Brick newBrick = createNewBrick(bricks[i], i);

        // handle node-brick
        newBrick.nodePos = nodePosCounter;

        // handle output-brick
        if(newBrick.isOutputBrick)
        {
            Node* nodes = segment.nodes;
            for(uint32_t j = 0; j < newBrick.numberOfNodes; j++) {
                nodes[j + nodePosCounter].border = -2.0f;
            }

            for(uint32_t i = 0; i < segment.segmentHeader->outputs.count; i++) {
                segment.outputs[i].targetNode = nodePosCounter + i;
            }

            newBrick.numberOfNodes = segment.segmentHeader->outputs.count;
        }

        // handle input-brick
        if(newBrick.isInputBrick)
        {
            Node* array = segment.nodes;
            for(uint32_t j = 0; j < newBrick.numberOfNodes; j++)
            {
                array[j + nodePosCounter].border = 0.0f;
                segment.inputs[inputCounter].targetNode = j + nodePosCounter;
                inputCounter++;
            }
        }

        Node* nodes = segment.nodes;
        for(uint32_t j = 0; j < newBrick.numberOfNodes; j++) {
            nodes[j + nodePosCounter].nodeBrickId = newBrick.nodeBrickId;
        }

        // copy new brick to segment
        segment.bricks[nodeBrickIdCounter] = newBrick;
        assert(nodeBrickIdCounter == newBrick.nodeBrickId);
        nodeBrickIdCounter++;
        nodePosCounter += newBrick.numberOfNodes;
    }

    return;
}

void
connectBrick(Segment &segment,
             Brick* sourceBrick,
             const uint8_t side)
{
    sourceBrick->neighbors[side] = UNINIT_STATE_32;
    Position next = getNeighborPos(sourceBrick->brickPos, side);
    if(next.isValid())
    {
        for(uint32_t t = 0; t < segment.segmentHeader->bricks.count; t++)
        {
            Brick* targetBrick = &segment.bricks[t];
            if(targetBrick->brickPos == next)
            {
                sourceBrick->neighbors[side] = targetBrick->brickId;
                targetBrick->neighbors[11 - side] = sourceBrick->brickId;
            }
        }
    }
}

/**
 * @brief connect all bricks in the parser-output based on its coordinates to identify neighbors
 *
 * @param parserOutput output coming from the parser
 * @param x current x-position
 * @param y current y-position
 * @param z current z-position
 */
void
connectAllBricks(Segment &segment)
{
    for(uint32_t i = 0; i < segment.segmentHeader->bricks.count; i++)
    {
        Brick* sourceBrick = &segment.bricks[i];
        for(uint8_t side = 0; side < 12; side++) {
            connectBrick(segment, sourceBrick, side);
        }
    }
}

/**
 * @brief get the next position in the raster for a specific brick and side
 *
 * @param x current x-position
 * @param y current y-position
 * @param z current z-position
 * @param side side to go to next
 *
 * @return position of the next brick for the specific side
 */
Position
getNeighborPos(Position sourcePos, const uint8_t side)
{
    Position result;
    result.x = UNINIT_STATE_32;
    result.y = UNINIT_STATE_32;
    result.z = UNINIT_STATE_32;

    switch (side)
    {
    case 0:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z - 1;
            break;
        }
    case 1:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z - 1;
            break;
        }
    case 2:
        {
            result.x = sourcePos.x;
            result.y = sourcePos.y;
            result.z = sourcePos.z - 1;
            break;
        }
    case 3:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z;
            break;
        }
    case 4:
        {
            result.x = sourcePos.x + 1;
            result.y = sourcePos.y;
            result.z = sourcePos.z;
            break;
        }
    case 5:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z;
            break;
        }
    case 8:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z;
            break;
        }
    case 7:
        {
            result.x = sourcePos.x - 1;
            result.y = sourcePos.y;
            result.z = sourcePos.z;
            break;
        }
    case 6:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y - 1;
            result.z = sourcePos.z;
            break;
        }
    case 9:
        {
            result.x = sourcePos.x;
            result.y = sourcePos.y;
            result.z = sourcePos.z + 1;
            break;
        }
    case 10:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x;
            } else {
                result.x = sourcePos.x - 1;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z + 1;
            break;
        }
    case 11:
        {
            if(sourcePos.y % 2 == 1) {
                result.x = sourcePos.x + 1;
            } else {
                result.x = sourcePos.x;
            }
            result.y = sourcePos.y + 1;
            result.z = sourcePos.z + 1;
            break;
        }
    default:
        assert(false);
    }
    return result;
}


bool
initializeAxons(Segment &segment)
{
    uint32_t nodeId = 0;

    // calculate number of axons per brick
    for(uint32_t brickId = 0;
        brickId < segment.segmentHeader->bricks.count;
        brickId++)
    {
        Brick* sourceBrick = &segment.bricks[brickId];

        if(sourceBrick->isOutputBrick == 1)
        {
            nodeId += sourceBrick->numberOfNodes;
            continue;
        }

        // iterate over all nodes of the brick and create an axon for each node
        for(uint32_t nodePos = 0; nodePos < sourceBrick->numberOfNodes; nodePos++)
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
            segment.nodes[nodeId].targetBrickDistance = static_cast<uint32_t>(dist);
            segment.nodes[nodeId].targetSectionId = nodeId;

            // post-check
            assert(axonBrick->nodeBrickId != UNINIT_STATE_32);
            assert(sourceBrick->brickId != UNINIT_STATE_32);

            nodeId++;
        }
    }

    assert(nodeId == segment.segmentHeader->nodes.count);

    return true;
}

/**
 * @brief FanBrickInitializer::getAxonBrick
 * @param segment
 * @param sourceBrick
 * @return
 */
Brick*
getAxonBrick(Segment &segment, Brick *sourceBrick)
{
    return sourceBrick;
}

/**
 * @brief FanBrickInitializer::initTargetBrickList
 * @param segment
 * @return
 */
bool
initTargetBrickList(Segment &segment,
                    InitSettings* init)
{
    Brick* bricks = segment.bricks;

    // iterate over all bricks
    for(uint32_t i = 0; i < segment.segmentHeader->bricks.count; i++)
    {
        Brick* baseBrick = &bricks[i];
        if(baseBrick->isOutputBrick != 0) {
            continue;
        }

        // get 1000 samples
        uint32_t counter = 0;
        while(counter < 1000)
        {
            baseBrick->possibleTargetNodeBrickIds[counter] = baseBrick->brickId + 1;
            counter++;
            /*uint8_t nextSide = getPossibleNext();
            const uint32_t nextBrickId = baseBrick->neighbors[nextSide];
            if(nextBrickId != UNINIT_STATE_32)
            {
                baseBrick->possibleTargetNodeBrickIds[counter] = nextBrickId;
                counter++;
                if(counter >= 1000) {
                    break;
                }
            }*/
        }
        assert(counter == 1000);
    }

    return true;
}
