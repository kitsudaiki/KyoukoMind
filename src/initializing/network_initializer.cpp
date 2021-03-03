/**
 * @file        network_initializer.cpp
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

#include "network_initializer.h"
#include <kyouko_root.h>

#include <core/objects/segment.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

#include <core/processing/processing_unit_handler.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <libKitsunemimiPersistence/logger/logger.h>

#include <libKitsunemimiAiParser/ai_parser_input.h>

/**
 * @brief constructor
 */
NetworkInitializer::NetworkInitializer() {}

/**
 * @brief create blank new network
 *
 * @param fileContent file to parse with the basic structure of the network
 *
 * @return true, if successfull, else false
 */
bool
NetworkInitializer::createNewNetwork(const std::string &fileContent,
                                     const std::string &configFileContent)
{
    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(fileContent == "") {
        return false;
    }

    // parse input
    std::string errorMessage = "";
    Kitsunemimi::Ai::AiBaseMeta parsedContent;
    Kitsunemimi::Ai::AiParserInput inputParser;
    const bool ret = inputParser.parseAi(parsedContent, fileContent, configFileContent, errorMessage);
    if(ret == false)
    {
        LOG_ERROR("error while parsing input: " + errorMessage);
        return false;
    }

    const uint32_t numberOfBricks = static_cast<uint32_t>(parsedContent.bricks.size());

    GlobalValues globalValues;
    globalValues.nodesPerBrick = parsedContent.initializingMeta.nodesPerBrick;
    globalValues.maxBrickDistance = parsedContent.initializingMeta.maxBrickDistance;
    globalValues.nodeLowerBorder = parsedContent.initializingMeta.nodeLowerBorder;
    globalValues.nodeUpperBorder = parsedContent.initializingMeta.nodeUpperBorder;
    globalValues.maxSynapseSections = parsedContent.initializingMeta.maxSynapseSections;

    globalValues.synapseDeleteBorder = parsedContent.processingMeta.synapseDeleteBorder;
    globalValues.actionPotential = parsedContent.processingMeta.actionPotential;
    globalValues.nodeCooldown = parsedContent.processingMeta.nodeCooldown;
    globalValues.memorizing = parsedContent.processingMeta.memorizing;
    globalValues.gliaValue = parsedContent.processingMeta.gliaValue;
    globalValues.maxSynapseWeight = parsedContent.processingMeta.maxSynapseWeight;
    globalValues.refractionTime = parsedContent.processingMeta.refractionTime;

    // update message for the monitoring
    KyoukoRoot::monitoringBrickMessage.numberOfInfos = numberOfBricks;

    // init segment
    const uint32_t numberOfNodeBricks = parsedContent.numberOfNodeBricks;
    const uint32_t totalNumberOfNodes = numberOfNodeBricks * globalValues.nodesPerBrick;

    // init segment
    Segment* segment = KyoukoRoot::m_segment;
    segment->nodesPerBrick = globalValues.nodesPerBrick;
    if(segment->initializeBuffer(numberOfBricks,
                                 parsedContent.numberOfNodeBricks,
                                 totalNumberOfNodes,
                                 globalValues.maxSynapseSections,
                                 parsedContent.numberOfOutputBricks,
                                 3,
                                 1024) == false)
    {
        return false;
    }

    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(segment->globalValues);
    *globalValue = globalValues;

    // fill array with empty nodes
    Node* array = Kitsunemimi::getBuffer<Node>(segment->nodes);
    for(uint32_t i = 0; i < totalNumberOfNodes; i++) {
        array[i].border = fmod((float)rand(), (globalValue->nodeUpperBorder - globalValue->nodeLowerBorder)) + globalValue->nodeLowerBorder;
    }

    addBricks(*segment, parsedContent);
    initTargetBrickList(*segment);

    SynapseSection* section = Kitsunemimi::getBuffer<SynapseSection>(segment->synapses);
    Brick** nodeBricks = segment->nodeBricks;
    for(uint32_t i = 0; i < totalNumberOfNodes; i++)
    {
        section[i].status = Kitsunemimi::ItemBuffer::ACTIVE_SECTION;
        section[i].randomPos = rand() % 1024;
        const uint32_t nodeBrickPos = rand() % segment->numberOfNodeBricks;
        section[i].nodeBrickId = nodeBricks[nodeBrickPos]->nodeBrickId;
        assert(section[i].nodeBrickId < segment->numberOfNodeBricks);
    }

    // init axons
    if(createAxons(*segment) == false) {
        return false;
    }

    return true;
}
/**
 * @brief add new brick
 *
 * @param segment segment where to add a new brick
 */
void
NetworkInitializer::addBricks(Segment &segment,
                              const Kitsunemimi::Ai::AiBaseMeta& metaBase)
{
    GlobalValues* globalValues = Kitsunemimi::getBuffer<GlobalValues>(segment.globalValues);

    for(uint32_t i = 0; i < metaBase.bricks.size(); i++)
    {
        Brick brick;

        // copy metadata
        brick.brickId = metaBase.bricks[i].brickId;
        brick.nodeBrickId = metaBase.bricks[i].nodeBrickId;
        brick.isOutputBrick = metaBase.bricks[i].isOutputBrick;
        brick.isInputBrick = metaBase.bricks[i].isInputBrick;

        // copy position
        brick.brickPos.x = static_cast<int32_t>(metaBase.bricks[i].brickPos.x);
        brick.brickPos.y = static_cast<int32_t>(metaBase.bricks[i].brickPos.y);
        brick.brickPos.z = static_cast<int32_t>(metaBase.bricks[i].brickPos.z);

        // copy neighbors
        for(uint32_t j = 0; j < 12; j++) {
            brick.neighbors[j] = metaBase.bricks[i].neighbors[j];
        }

        // handle node-brick
        if(brick.nodeBrickId != UNINIT_STATE_32)
        {
            const uint32_t nodePos = brick.nodeBrickId * globalValues->nodesPerBrick;
            assert(nodePos < 0x7FFFFFFF);
            brick.nodePos = nodePos;

            // handle output-brick
            if(brick.isOutputBrick)
            {
                Node* array = Kitsunemimi::getBuffer<Node>(segment.nodes);
                for(uint32_t j = 0; j < globalValues->nodesPerBrick; j++) {
                    array[j + nodePos].border = -2.0f;
                }
            }

            // handle input-brick
            if(brick.isInputBrick)
            {
                Node* array = Kitsunemimi::getBuffer<Node>(segment.nodes);
                for(uint32_t j = 0; j < globalValues->nodesPerBrick; j++) {
                    array[j + nodePos].border = 0.0f;
                }
            }
        }

        assert(brick.brickId == i);
        // copy new brick to segment
        Kitsunemimi::getBuffer<Brick>(segment.bricks)[i] = brick;

        // link if node-brick
        if(brick.nodeBrickId != UNINIT_STATE_32)
        {
            assert(brick.nodeBrickId < segment.numberOfNodeBricks);
            segment.nodeBricks[brick.nodeBrickId] = &Kitsunemimi::getBuffer<Brick>(segment.bricks)[i];
        }

        // link output-brick
        if(brick.isOutputBrick)
        {
            Brick* outputBrickPtr = &Kitsunemimi::getBuffer<Brick>(segment.bricks)[i];
            segment.outputBricks[segment.numberOfOutputBricks] = outputBrickPtr;
            segment.numberOfOutputBricks++;
        }

        // link input-brick
        if(brick.isInputBrick)
        {
            Brick* inputBrickPtr = &Kitsunemimi::getBuffer<Brick>(segment.bricks)[i];
            segment.inputBricks[segment.numberOfInputBricks] = inputBrickPtr;
            segment.numberOfInputBricks++;
        }
    }

    return;
}

/**
 * @brief create new axons
 *
 * @param segment segment with the basic data
 * @param networkMetaStructure reference to resulting meta-structure
 *
 * @return true, if successfull, else false
 */
bool
NetworkInitializer::createAxons(Segment &segment)
{
    GlobalValues* globalValues = Kitsunemimi::getBuffer<GlobalValues>(segment.globalValues);
    Brick* bricks = Kitsunemimi::getBuffer<Brick>(segment.bricks);
    Node* nodes = Kitsunemimi::getBuffer<Node>(segment.nodes);

    // calculate number of axons per brick
    for(uint32_t i = 0; i < segment.bricks.numberOfItems; i++)
    {
        if(bricks[i].nodeBrickId == UNINIT_STATE_32) {
            continue;
        }

        Brick* sourceBrick = &bricks[i];
        const uint32_t pos = sourceBrick->nodeBrickId * globalValues->nodesPerBrick;

        // iterate over all nodes of the brick and create an axon for each node
        for(uint32_t nodePos = 0; nodePos < globalValues->nodesPerBrick; nodePos++)
        {
            Brick* axonBrick = nullptr;

            do {
                // get random brick as target for the axon
                const uint32_t randPos = static_cast<uint32_t>(rand()) % segment.bricks.numberOfItems;
                axonBrick = &bricks[randPos];
            }
            while(sourceBrick->isInputBrick
                  && axonBrick->isOutputBrick);

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
            nodes[pos + nodePos].nodeBrickId = sourceBrick->nodeBrickId;
            nodes[pos + nodePos].targetBrickDistance = static_cast<uint32_t>(dist);

            // post-check
            assert(axonBrick->brickId != UNINIT_STATE_32);
            assert(sourceBrick->brickId != UNINIT_STATE_32);
        }
    }

    return true;
}

/**
 * @brief NetworkInitializer::initTargetBrickList
 * @param segment
 * @return
 */
bool
NetworkInitializer::initTargetBrickList(Segment &segment)
{
    Brick* bricks = Kitsunemimi::getBuffer<Brick>(segment.bricks);
    GlobalValues* globalValues = Kitsunemimi::getBuffer<GlobalValues>(segment.globalValues);

    // iterate over all bricks
    for(uint32_t i = 0; i < segment.bricks.numberOfItems; i++)
    {
        if(bricks[i].nodeBrickId == UNINIT_STATE_32) {
            continue;
        }

        Brick* baseBrick = &bricks[i];

        // get 1000 samples
        uint32_t counter = 0;
        while(counter < 1000)
        {
            Brick jumpBrick = *baseBrick;

            // try to go a specific distance
            uint8_t nextSide = 42;
            for(uint32_t k = 0; k < globalValues->maxBrickDistance; k++)
            {
                nextSide = getPossibleNext(nextSide);
                const uint32_t nextBrickId = jumpBrick.neighbors[nextSide];
                if(nextBrickId != UNINIT_STATE_32)
                {
                    jumpBrick = bricks[nextBrickId];
                    if(jumpBrick.nodeBrickId != UNINIT_STATE_32)
                    {
                        // reject direct connections between input and output
                        if(jumpBrick.isOutputBrick != 0
                                && baseBrick->isInputBrick != 0)
                        {
                            continue;
                        }

                        if(jumpBrick.isInputBrick != 0) {
                            continue;
                        }

                        baseBrick->possibleTargetNodeBrickIds[counter] = jumpBrick.nodeBrickId;

                        // update and check counter
                        counter++;
                        if(counter >= 1000) {
                            break;
                        }
                    }
                    nextSide = 11 - nextSide;
                }
                else
                {
                    break;
                }
            }
        }
        assert(counter == 1000);
    }

    return true;
}

/**
 * @brief Brick::getPossibleNext
 * @param inputSide
 * @return
 */
uint8_t
NetworkInitializer::getPossibleNext(const uint8_t inputSide)
{
    uint8_t possibleNext[5];

    switch(inputSide)
    {
        case 0:
        {
            possibleNext[0] = 4;
            possibleNext[1] = 5;
            possibleNext[2] = 7;
            possibleNext[3] = 8;
            possibleNext[4] = 11;
            break;
        }

        case 1:
        {
            possibleNext[0] = 5;
            possibleNext[1] = 6;
            possibleNext[2] = 7;
            possibleNext[3] = 8;
            possibleNext[4] = 10;
            break;
        }

        case 2:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 5;
            possibleNext[3] = 6;
            possibleNext[4] = 9;
            break;
        }

        case 3:
        {
            possibleNext[0] = 5;
            possibleNext[1] = 7;
            possibleNext[2] = 8;
            possibleNext[3] = 2;
            possibleNext[4] = 11;
            break;
        }
        case 4:
        {
            possibleNext[0] = 6;
            possibleNext[1] = 7;
            possibleNext[2] = 8;
            possibleNext[3] = 2;
            possibleNext[4] = 10;
            break;
        }
        case 5:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 6;
            possibleNext[2] = 7;
            possibleNext[3] = 0;
            possibleNext[4] = 10;
            break;
        }
        case 6:
        {
            possibleNext[0] = 4;
            possibleNext[1] = 5;
            possibleNext[2] = 8;
            possibleNext[3] = 1;
            possibleNext[4] = 11;
            break;
        }
        case 7:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 5;
            possibleNext[3] = 1;
            possibleNext[4] = 9;
            break;
        }
        case 8:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 6;
            possibleNext[3] = 0;
            possibleNext[4] = 9;
            break;
        }
        case 9:
        {
            possibleNext[0] = 5;
            possibleNext[1] = 6;
            possibleNext[2] = 7;
            possibleNext[3] = 8;
            possibleNext[4] = 2;
            break;
        }
        case 10:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 5;
            possibleNext[3] = 8;
            possibleNext[4] = 1;
            break;
        }
        case 11:
        {
            possibleNext[0] = 3;
            possibleNext[1] = 4;
            possibleNext[2] = 6;
            possibleNext[3] = 7;
            possibleNext[4] = 0;
            break;
        }

        default:
        {
            return rand() % 12;
        }
    }

    return possibleNext[rand() % 5];
}

