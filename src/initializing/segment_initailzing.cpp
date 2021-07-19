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
 * @brief createNewHeader
 * @param numberOfBricks
 * @param numberOfNodes
 * @param numberOfSynapseSections
 * @param numberOfInputs
 * @param numberOfOutputs
 * @return
 */
SegmentHeader
createNewHeader(const uint32_t numberOfBricks,
                const uint32_t numberOfNodes,
                const uint64_t numberOfSynapseSections,
                const uint32_t numberOfInputs,
                const uint32_t numberOfOutputs)
{
    SegmentHeader segmentHeader;
    uint32_t persistentBufferPos = 0;

    // init header
    persistentBufferPos += 1 * sizeof(SegmentHeader);

    // init settings
    segmentHeader.settings.count = 1;
    segmentHeader.settings.bytePos = persistentBufferPos;
    persistentBufferPos += 1 * sizeof(SegmentSettings);

    // init bricks
    segmentHeader.bricks.count = numberOfBricks;
    segmentHeader.bricks.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfBricks * sizeof(Brick);

    // init brick-order
    segmentHeader.brickOrder.count = numberOfBricks;
    segmentHeader.brickOrder.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfBricks * sizeof(uint32_t);

    // init nodes
    segmentHeader.nodes.count = numberOfNodes;
    segmentHeader.nodes.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfNodes * sizeof(Node);

    // init synapse sections
    segmentHeader.synapseSections.count = numberOfSynapseSections;
    segmentHeader.synapseSections.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfSynapseSections * sizeof(SynapseSection);

    // init input
    segmentHeader.inputs.count = numberOfInputs;
    segmentHeader.inputs.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfInputs * sizeof(InputNode);

    // init output
    segmentHeader.outputs.count = numberOfOutputs;
    segmentHeader.outputs.bytePos = persistentBufferPos;
    persistentBufferPos += numberOfOutputs * sizeof(OutputNode);


    segmentHeader.segmentSize = persistentBufferPos;
    segmentHeader.segmentPersistentBufferSize = persistentBufferPos;

    return segmentHeader;
}

/**
 * @brief initSegmentPointer
 * @param segment
 * @param header
 */
void
initSegmentPointer(Segment &segment,
                   const SegmentHeader &header)
{
    uint8_t* persistentData = static_cast<uint8_t*>(segment.persistenBuffer.data);

    segment.segmentHeader = reinterpret_cast<SegmentHeader*>(persistentData + 0);
    segment.segmentHeader[0] = header;

    segment.synapseSettings = reinterpret_cast<SegmentSettings*>(persistentData + 256);
    segment.bricks = reinterpret_cast<Brick*>(persistentData + segment.segmentHeader->bricks.bytePos);
    segment.brickOrder = reinterpret_cast<uint32_t*>(persistentData + segment.segmentHeader->brickOrder.bytePos);
    segment.nodes = reinterpret_cast<Node*>(persistentData + segment.segmentHeader->nodes.bytePos);
    segment.synapseSections = reinterpret_cast<SynapseSection*>(persistentData + segment.segmentHeader->synapseSections.bytePos);
    segment.inputs = reinterpret_cast<InputNode*>(persistentData + segment.segmentHeader->inputs.bytePos);
    segment.outputs = reinterpret_cast<OutputNode*>(persistentData + segment.segmentHeader->outputs.bytePos);
}

/**
 * @brief allocateSegment
 * @param header
 * @return
 */
Segment*
allocateSegment(SegmentHeader &header)
{
    Segment* newSegment = new Segment();

    // persistent part
    const uint32_t numberOfPersistentBlocks = (header.segmentPersistentBufferSize / 4096) + 1;
    header.segmentPersistentBufferSize = numberOfPersistentBlocks * 4096;
    Kitsunemimi::allocateBlocks_DataBuffer(newSegment->persistenBuffer, numberOfPersistentBlocks);

    initSegmentPointer(*newSegment, header);

    return newSegment;
}

/**
 * @brief Segment::initSynapseSegment
 * @param numberOfNodeBricks
 * @param numberOfNodes
 * @param numberOfSynapseSections
 * @param numberOfRandValues
 * @return
 */
Segment*
createNewSegment(const uint32_t numberOfBricks,
                 const uint32_t numberOfNodes,
                 const uint64_t numberOfSynapseSections,
                 const uint32_t numberOfInputs,
                 const uint32_t numberOfOutputs,
                 const uint32_t numberOfRandValues)
{
    SegmentHeader header = createNewHeader(numberOfBricks,
                                           numberOfNodes,
                                           numberOfSynapseSections,
                                           numberOfInputs,
                                           numberOfOutputs);
    Segment* segment = allocateSegment(header);

    // init header and metadata
    segment->synapseSettings[0] = SegmentSettings();

    // init bricks;
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        segment->bricks[i] = Brick();
    }

    // init brick-order
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        segment->brickOrder[i] = i;
    }

    // init nodes
    for(uint32_t i = 0; i < numberOfNodes; i++) {
        segment->nodes[i] = Node();
    }

    // init synapse sections
    for(uint32_t i = 0; i < numberOfSynapseSections; i++)
    {
        segment->synapseSections[i] = SynapseSection();
        segment->synapseSections[i].randomPos = static_cast<uint32_t>(rand()) % numberOfRandValues;
    }

    // init input
    for(uint32_t i = 0; i < numberOfInputs; i++) {
        segment->inputs[i] = InputNode();
    }

    // init output
    for(uint32_t i = 0; i < numberOfOutputs; i++) {
        segment->outputs[i] = OutputNode();
    }

    return segment;
}

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

/**
 * @brief addBricksToSegment
 * @param segment
 * @param initMetaData
 * @param metaBase
 */
void
addBricksToSegment(Segment &segment,
                   InitSettings* initMetaData,
                   JsonItem &metaBase)
{
    uint32_t nodeBrickIdCounter = 0;
    uint32_t inputCounter = 0;
    JsonItem bricks = metaBase.get("bricks");

    for(uint32_t i = 0; i < bricks.size(); i++)
    {
        Brick newBrick;

        // copy metadata
        newBrick.brickId = i;
        newBrick.nodeBrickId = i;
        if(bricks[i].get("type").getString() == "output") {
            newBrick.isOutputBrick = true;
        }
        if(bricks[i].get("type").getString() == "input") {
            newBrick.isInputBrick = true;
        }

        // copy position
        newBrick.brickPos.x = bricks[i].get("x").getInt();
        newBrick.brickPos.y = bricks[i].get("y").getInt();
        newBrick.brickPos.z = bricks[i].get("z").getInt();

        // handle node-brick
        const uint32_t nodeOffset = newBrick.nodeBrickId * initMetaData->nodesPerBrick;
        assert(nodeOffset < 0x7FFFFFFF);
        newBrick.nodePos = nodeOffset;
        newBrick.numberOfNodes = initMetaData->nodesPerBrick;

        // handle output-brick
        if(newBrick.isOutputBrick)
        {
            Node* nodes = segment.nodes;
            for(uint32_t j = 0; j < initMetaData->nodesPerBrick; j++) {
                nodes[j + nodeOffset].border = -2.0f;
            }

            for(uint32_t i = 0; i < segment.segmentHeader->outputs.count; i++) {
                segment.outputs[i].targetNode = nodeOffset + i;
            }

            newBrick.numberOfNodes = segment.segmentHeader->outputs.count;
        }

        // handle input-brick
        if(newBrick.isInputBrick)
        {
            Node* array = segment.nodes;
            for(uint32_t j = 0; j < initMetaData->nodesPerBrick; j++)
            {
                array[j + nodeOffset].border = 0.0f;
                segment.inputs[inputCounter].targetNode = j + nodeOffset;
                inputCounter++;
            }
        }

        Node* nodes = segment.nodes;
        for(uint32_t j = 0; j < initMetaData->nodesPerBrick; j++) {
            nodes[j + nodeOffset].nodeBrickId = newBrick.nodeBrickId;
        }

        // copy new brick to segment
        segment.bricks[nodeBrickIdCounter] = newBrick;
        assert(nodeBrickIdCounter == newBrick.nodeBrickId);
        nodeBrickIdCounter++;
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
