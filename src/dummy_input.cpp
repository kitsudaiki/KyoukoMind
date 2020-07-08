/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <dummy_input.h>
#include <kyouko_root.h>

#include <core/processing/external/message_processing.h>
#include <core/processing/objects/container_definitions.h>

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
    inputBrick->initCycle();

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
        Brick::Neighbor* neighbor = &inputBrick->neighbors[22];

        // set input-values
        EdgeContainer newEdge;
        newEdge.weight = 300;
        newEdge.targetEdgeSectionId = 1;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
        /*newEdge.targetEdgeSectionId = 2;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
        newEdge.targetEdgeSectionId = 3;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
        newEdge.targetEdgeSectionId = 4;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
        newEdge.targetEdgeSectionId = 5;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
        newEdge.targetEdgeSectionId = 6;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
        newEdge.targetEdgeSectionId = 7;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
        newEdge.targetEdgeSectionId = 8;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
        newEdge.targetEdgeSectionId = 9;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
        newEdge.targetEdgeSectionId = 10;
        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);*/

        // finish and restart cycle
        inputBrick->finishSide(22);
        while(inputBrick->isReady() == false) {
            usleep(1000);
        }
        inputBrick->initCycle();
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
    assert(inputBrick->connectBricks(sourceSide, *targetBrick));
    KyoukoRoot::m_inputBricks->insert(std::make_pair(0, inputBrick));
}

}
