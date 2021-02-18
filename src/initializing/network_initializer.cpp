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
#include <core/objects/global_values.h>

#include <core/processing/processing_unit_handler.h>
#include <core/processing/gpu/gpu_processing_uint.h>
#include <initializing/segment_initializing.h>

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
NetworkInitializer::createNewNetwork(const std::string &fileContent)
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
    const bool ret = inputParser.parseAi(parsedContent, fileContent, errorMessage);
    if(ret == false)
    {
        LOG_ERROR("error while parsing input: " + errorMessage);
        return false;
    }

    const uint32_t numberOfBricks = static_cast<uint32_t>(parsedContent.bricks.size());

    // update message for the monitoring
    KyoukoRoot::monitoringBrickMessage.numberOfInfos = numberOfBricks;

    // init segment
    Segment* segment = KyoukoRoot::m_segment;
    const uint32_t numberOfNodeBricks = parsedContent.numberOfNodeBricks;
    GlobalValues* globalValues = getBuffer<GlobalValues>(segment->globalValues);
    const uint32_t totalNumberOfNodes = numberOfNodeBricks * globalValues->numberOfNodesPerBrick;

    if(initNodeBlocks(*segment, totalNumberOfNodes) == false) {
        return false;
    }

    if(initRandomValues(*segment) == false) {
        return false;
    }

    // init bricks
    initBricks(*segment, numberOfBricks);
    segment->nodeBricks = new Brick*[parsedContent.numberOfNodeBricks];
    segment->inputBricks = new Brick*[parsedContent.numberOfNodeBricks];
    segment->outputBricks = new Brick*[parsedContent.numberOfNodeBricks];
    segment->numberOfNodeBricks = parsedContent.numberOfNodeBricks;
    addBricks(*segment, parsedContent);

    // init axons
    createAxons(*segment);

    // init synapses
    if(initSynapseSectionBlocks(*segment,
                                MAX_NUMBER_OF_SYNAPSE_SECTIONS,
                                totalNumberOfNodes) == false)
    {
        return false;
    }

    // mark all synapses als delted to make them usable
    if(segment->synapses.deleteAll() == false) {
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
    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

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
        for(uint32_t i = 0; i < 12; i++) {
            brick.neighbors[i] = metaBase.bricks[i].neighbors[i];
        }

        // handle node-brick
        if(brick.nodeBrickId != UNINIT_STATE_32)
        {
            const uint32_t nodePos = brick.nodeBrickId * globalValues->numberOfNodesPerBrick;
            assert(nodePos < 0x7FFFFFFF);
            brick.nodePos = nodePos;

            // handle output-brick
            if(brick.isOutputBrick)
            {
                Node* array = getBuffer<Node>(segment.nodes);
                for(uint32_t i = 0; i < globalValues->numberOfNodesPerBrick; i++) {
                    array[i + nodePos].border = -2.0f;
                }
            }

            // handle input-brick
            if(brick.isInputBrick)
            {
                Node* array = getBuffer<Node>(segment.nodes);
                for(uint32_t i = 0; i < globalValues->numberOfNodesPerBrick; i++) {
                    array[i + nodePos].border = 0.0f;
                }
            }
        }

        assert(brick.brickId == i);
        // copy new brick to segment
        getBuffer<Brick>(segment.bricks)[i] = brick;

        // link if node-brick
        if(brick.nodeBrickId != UNINIT_STATE_32)
        {
            assert(brick.nodeBrickId < segment.numberOfNodeBricks);
            segment.nodeBricks[brick.nodeBrickId] = &getBuffer<Brick>(segment.bricks)[i];
        }

        // link output-brick
        if(brick.isOutputBrick)
        {
            Brick* outputBrickPtr = &getBuffer<Brick>(segment.bricks)[i];
            segment.outputBricks[segment.numberOfOutputBricks] = outputBrickPtr;
            segment.numberOfOutputBricks++;
        }

        // link input-brick
        if(brick.isInputBrick)
        {
            Brick* inputBrickPtr = &getBuffer<Brick>(segment.bricks)[i];
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
    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    Brick* bricks = getBuffer<Brick>(KyoukoRoot::m_segment->bricks);
    Node* nodes = getBuffer<Node>(segment.nodes);

    // calculate number of axons per brick
    for(uint32_t i = 0; i < KyoukoRoot::m_segment->bricks.numberOfItems; i++)
    {
        if(bricks[i].nodeBrickId == UNINIT_STATE_32) {
            continue;
        }

        Brick* sourceBrick = &bricks[i];
        const uint32_t pos = sourceBrick->nodeBrickId * globalValues->numberOfNodesPerBrick;

        // iterate over all nodes of the brick and create an axon for each node
        for(uint32_t nodePos = 0; nodePos < globalValues->numberOfNodesPerBrick; nodePos++)
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
            nodes[pos + nodePos].brickId = sourceBrick->brickId;
            nodes[pos + nodePos].targetBrickDistance = static_cast<uint32_t>(dist);

            // post-check
            assert(axonBrick->brickId != UNINIT_STATE_32);
            assert(sourceBrick->brickId != UNINIT_STATE_32);
        }
    }

    return true;
}

