/**
 *  @file    dummy_input.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#include <dummy_input.h>
#include <kyouko_root.h>

#include <core/methods/brick_initializing_methods.h>
#include <core/processing/methods/brick_processing.h>
#include <core/processing/methods/edge_container_processing.h>
#include <core/objects/container_definitions.h>

#include <libKitsunemimiPersistence/logger/logger.h>

namespace KyoukoMind
{

DummyInput::DummyInput()
{

}

void
DummyInput::run()
{
    initialize();

    while(!m_abort)
    {
        // set global values
        GlobalValuesHandler* handler = KyoukoRoot::m_globalValuesHandler;
        GlobalValues gValues = handler->getGlobalValues();
        gValues.globalGlia = m_gliaValue;
        gValues.globalMemorizingTemp = m_memTemp;
        gValues.globalMemorizingOffset = m_memOffset;
        gValues.globalLearningTemp = m_learnTemp;
        gValues.globalLearningOffset = m_learnOffset;
        handler->setGlobalValues(gValues);

        // get neighbor
        Neighbor* neighbor = &inputBrick->neighbors[22];
        const uint16_t ok = neighbor->targetBrick->nodePos >= 0;

        // set input-values
        for(uint16_t i = 0; i < ok * NUMBER_OF_NODES_PER_BRICK; i++)
        {
            assert(neighbor->outgoingBuffer != nullptr);

            EdgeContainer newEdge;
            newEdge.weight = m_inputValue;
            newEdge.targetEdgeSectionId = i;
            assert(neighbor->currentBuffer != nullptr);
            Kitsunemimi::addObject_StackBuffer(*neighbor->currentBuffer, &newEdge);
        }

        // finish and reint next cycle
        processIncomingMessages2(*KyoukoRoot::m_segment, *inputBrick, 22);
        assert(inputBrick->neighbors[22].outgoingBuffer != nullptr);
        finishSide(inputBrick, 22);
        while(isReady(inputBrick) == false) {
            usleep(1000);
        }
        initCycle(inputBrick);

        // sleep until next cycle
        sleepThread(100000);
    }
}

/**
 * @brief initalize test-input
 */
void
DummyInput::initialize()
{
    std::string errorMessage = "";
    const uint32_t fakeId = 10000;
    const uint8_t sourceSide = 22;

    // check if target-brick, which is specified by the id in the messge, does exist
    Brick* targetBrick = KyoukoRoot::m_segment->bricks.at(0);
    assert(targetBrick != nullptr);

    // create fake-brick to connect it with the defined input-brick
    inputBrick = new Brick(fakeId, 0, 0);
    inputBrick->isInputBrick = 1;

    // init the new neighbors
    assert(connectBricks(*inputBrick, sourceSide, *targetBrick));
    KyoukoRoot::m_inputBricks->insert(std::make_pair(0, inputBrick));

    initEdgeSectionBlocks(*inputBrick, NUMBER_OF_NODES_PER_BRICK);
    initRandValues(*inputBrick);
    initCycle(inputBrick);
}

}
