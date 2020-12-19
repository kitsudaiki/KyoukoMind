/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "brick.h"

#include <core/objects/segment.h>
#include <core/objects/node.h>
#include <core/objects/edges.h>
#include <core/objects/global_values.h>

#include <libKitsunemimiKyoukoCommon/communication_structs/monitoring_contianer.h>
#include <libKitsunemimiKyoukoCommon/communication_structs/mind_container.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief Brick::Brick
 */
Brick::Brick()
{
    initNeighborList();
}

/**
 * @brief Brick::Brick
 * @param brickId
 * @param x
 * @param y
 */
Brick::Brick(const uint32_t &brickId,
             const uint32_t x,
             const uint32_t y)
{
    this->brickId = brickId;
    this->brickPos.x = x;
    this->brickPos.y = y;

    updateMonitoringMetadata();
    initNeighborList();
}

/**
 * @brief Brick::Brick
 * @param other
 */
Brick::Brick(const Brick &other)
{
    if(this != &other)
    {
        this->brickId = other.brickId;
        this->nodeBrickId = other.nodeBrickId;
        this->brickPos = other.brickPos;
        this->nodePos = other.nodePos;
        this->nodeActivity = other.nodeActivity;
        this->synapseActivity = other.synapseActivity;
        this->edgeCreateActivity = other.edgeCreateActivity;
        this->edgeDeleteActivity = other.edgeDeleteActivity;
        this->synapseCreateActivity = other.synapseCreateActivity;
        this->synapseDeleteActivity = other.synapseDeleteActivity;

        for(uint32_t i = 0; i < 23; i++) {
            this->neighbors[i] = other.neighbors[i];
        }
    }
}

/**
 * @brief Brick::operator =
 * @param other
 * @return
 */
Brick
&Brick::operator=(const Brick &other)
{
    if(this != &other)
    {
        this->brickId = other.brickId;
        this->nodeBrickId = other.nodeBrickId;
        this->brickPos = other.brickPos;
        this->nodePos = other.nodePos;
        this->nodeActivity = other.nodeActivity;
        this->synapseActivity = other.synapseActivity;
        this->edgeCreateActivity = other.edgeCreateActivity;
        this->edgeDeleteActivity = other.edgeDeleteActivity;
        this->synapseCreateActivity = other.synapseCreateActivity;
        this->synapseDeleteActivity = other.synapseDeleteActivity;

        for(uint32_t i = 0; i < 23; i++) {
            this->neighbors[i] = other.neighbors[i];
        }
    }

    return *this;
}

/**
 * @brief Brick::~Brick
 */
Brick::~Brick() {}

/**
 * @brief Brick::getRandomNeighbor
 * @return
 */
uint32_t
Brick::getRandomNeighbor(const uint32_t location)
{
    const uint8_t inputSide = getInputSide(location);
    const PossibleNext next = getPossibleNext(inputSide);

    const uint8_t nextSide = next.next[rand() % 3];
    uint32_t nextLocation = neighbors[nextSide];

    if(nextLocation != UNINIT_STATE_32) {
        nextLocation += static_cast<uint32_t>(23 - nextSide) << 24;
    }

    return nextLocation;
}

/**
 * @brief Brick::initNeighborList
 */
void
Brick::initNeighborList()
{
    for(uint8_t i = 0; i < 23; i++)
    {
        neighbors[i] = UNINIT_STATE_32;
    }
}

/**
 * connect two bricks by initialing the neighbors betweens the two bricks
 *
 * @return true, if successful, else false
 */
bool
Brick::connectBricks(const uint8_t sourceSide,
                     Brick &targetBrick)
{
    assert(sourceSide < 23);

    // check neighbors
    if(neighbors[sourceSide] != UNINIT_STATE_32
            || targetBrick.neighbors[23-sourceSide] != UNINIT_STATE_32)
    {
        return false;
    }

    // init the new neighbors
    this->initNeighbor(sourceSide,
                       targetBrick.brickId);
    targetBrick.initNeighbor(23 - sourceSide,
                             this->brickId);

    return true;
}

/**
 * remove the connection between two neighbors
 *
 * @return true, if successful, else false
 */
bool
Brick::disconnectBricks(const uint8_t sourceSide)
{
    assert(sourceSide < 23);
    const uint32_t targetId = neighbors[sourceSide];

    Brick* targetBrick = &getBuffer<Brick>(KyoukoRoot::m_segment->bricks)[targetId];

    // check neighbors
    if(neighbors[sourceSide] == UNINIT_STATE_32
            || targetBrick->neighbors[23-sourceSide] == UNINIT_STATE_32)
    {
        return false;
    }

    // add the new neighbor
    this->uninitNeighbor(sourceSide);
    targetBrick->uninitNeighbor(23 - sourceSide);

    return true;
}

/**
 * @brief Brick::registerInput
 * @return
 */
uint32_t
Brick::registerInput()
{
    while(m_input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    if(m_inputs.size() >= globalValues->numberOfNodesPerBrick - 10) {
        return UNINIT_STATE_32;
    }

    m_inputs.push_back(0.0f);
    m_inputs.push_back(0.0f);
    m_inputs.push_back(0.0f);
    m_inputs.push_back(0.0f);
    m_inputs.push_back(0.0f);
    m_inputs.push_back(0.0f);
    m_inputs.push_back(0.0f);
    m_inputs.push_back(0.0f);
    m_inputs.push_back(0.0f);
    m_inputs.push_back(0.0f);
    const uint32_t listPos = static_cast<uint32_t>(m_inputs.size() / 10) - 1;

    m_input_lock.clear(std::memory_order_release);

    return listPos;
}

/**
 * @brief Brick::setInputValue
 * @param pos
 * @param value
 */
void
Brick::setInputValue(const uint32_t pos, const float value)
{
    while(m_input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    //LOG_WARNING("input-value: " + std::to_string(value));
    for(uint32_t i = pos * 10; i < (pos * 10) + 10; i++) {
        m_inputs[i] = value;
    }
    m_input_lock.clear(std::memory_order_release);
}

/**
 * @brief Brick::getInputValues
 * @return
 */
const std::vector<float>
Brick::getInputValues()
{
    while(m_input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    const std::vector<float> copy = m_inputs;
    m_input_lock.clear(std::memory_order_release);
    return copy;
}

/**
 * @brief Brick::clearInput
 */
void
Brick::clearInput()
{
    while(m_input_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    for(uint32_t i = 0; i < m_inputs.size(); i++) {
        m_inputs[i] = 0.0f;
    }
    m_input_lock.clear(std::memory_order_release);
}

/**
 * @brief Brick::registerOutput
 * @return
 */
uint32_t
Brick::registerOutput()
{
    while(m_output_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    while(m_should_lock.test_and_set(std::memory_order_acquire)) { asm(""); }

    GlobalValues* globalValues = getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);
    if(m_outputs.size() >= globalValues->numberOfNodesPerBrick - 1) {
        return UNINIT_STATE_32;
    }

    m_outputs.push_back(0.0f);
    m_outputs.push_back(0.0f);
    m_outputs.push_back(0.0f);
    m_outputs.push_back(0.0f);
    m_outputs.push_back(0.0f);
    m_outputs.push_back(0.0f);
    m_outputs.push_back(0.0f);
    m_outputs.push_back(0.0f);
    m_outputs.push_back(0.0f);
    m_outputs.push_back(0.0f);

    m_should.push_back(0.0f);

    const uint32_t listPos = static_cast<uint32_t>(m_outputs.size()) - 1;
    m_should_lock.clear(std::memory_order_release);
    m_output_lock.clear(std::memory_order_release);

    return listPos;
}

/**
 * @brief Brick::setOutputValue
 * @param pos
 * @param value
 */
void
Brick::setOutputValue(const uint32_t pos, const float value)
{
    while(m_output_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    if(pos < m_outputs.size()) {
        m_outputs[pos] += value;
    }
    m_output_lock.clear(std::memory_order_release);
}

/**
 * @brief Brick::resetOutputValues
 */
void
Brick::resetOutputValues()
{
    while(m_output_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    for(uint32_t i = 0; i < m_outputs.size(); i++) {
        m_outputs[i] = 0.0f;
    }
    m_output_lock.clear(std::memory_order_release);
}

/**
 * @brief Brick::getNumberOfOutputValues
 * @return
 */
uint32_t
Brick::getNumberOfOutputValues()
{
    while(m_output_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    const uint32_t size = static_cast<uint32_t>(m_outputs.size());
    m_output_lock.clear(std::memory_order_release);
    return size;
}

/**
 * @brief Brick::getOutputValues
 * @return
 */
const std::vector<float>
Brick::getOutputValues()
{
    while(m_output_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    const std::vector<float> copy = m_outputs;
    m_output_lock.clear(std::memory_order_release);
    return copy;
}

/**
 * @brief Brick::setShouldValue
 * @param pos
 * @param value
 */
void
Brick::setShouldValue(const uint32_t pos, const float value)
{
    while(m_should_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    if(pos < m_should.size()) {
        m_should[pos] = value;
    }
    m_should_lock.clear(std::memory_order_release);
}

/**
 * @brief Brick::getShouldValues
 * @return
 */
const std::vector<float>
Brick::getShouldValues()
{
    while(m_should_lock.test_and_set(std::memory_order_acquire)) { asm(""); }
    const std::vector<float> copy = m_should;
    m_should_lock.clear(std::memory_order_release);
    return copy;
}

/**
 * uninitialize a specific neighbor of a brick
 *
 * @return true, if successful, else false
 */
bool
Brick::uninitNeighbor(const uint8_t side)
{
    if(neighbors[side] == UNINIT_STATE_32) {
        return false;
    }

    neighbors[side] = UNINIT_STATE_32;

    return true;
}

/**
 * @brief Brick::getPossibleNext
 * @param inputSide
 * @return
 */
const Brick::PossibleNext
Brick::getPossibleNext(const uint8_t inputSide)
{
    PossibleNext next;
    std::vector<uint8_t> possibleNext = { 9, 10, 11, 12, 13, 14 };

    switch(inputSide)
    {
        case 0:
        {
            next.next[0] = possibleNext[rand() % 6];
            next.next[1] = possibleNext[rand() % 6];
            next.next[2] = possibleNext[rand() % 6];
            break;
        }

        case 9:
        {
            next.next[0] = 11;
            next.next[1] = 13;
            next.next[2] = 14;
            break;
        }

        case 10:
        {
            next.next[0] = 12;
            next.next[1] = 13;
            next.next[2] = 14;
            break;
        }

        case 11:
        {
            next.next[0] = 9;
            next.next[1] = 12;
            next.next[2] = 13;
            break;
        }

        case 12:
        {
            next.next[0] = 10;
            next.next[1] = 11;
            next.next[2] = 14;
            break;
        }

        case 13:
        {
            next.next[0] = 9;
            next.next[1] = 10;
            next.next[2] = 11;
            break;
        }

        case 14:
        {
            next.next[0] = 9;
            next.next[1] = 10;
            next.next[2] = 12;
            break;
        }

        default:
        {
            next.next[0] = UNINIT_STATE_8;
            next.next[1] = UNINIT_STATE_8;
            next.next[2] = UNINIT_STATE_8;
        }
    }

    return next;
}

/**
 * @brief Brick::updateMonitoringMetadata
 */
void
Brick::updateMonitoringMetadata()
{
    KyoukoRoot::monitoringBrickMessage.brickInfos[brickId].xPos = brickPos.x;
    KyoukoRoot::monitoringBrickMessage.brickInfos[brickId].yPos = brickPos.y;
    KyoukoRoot::monitoringBrickMessage.brickInfos[brickId].brickId = brickId;
}

/**
 * @brief Brick::initNeighbor
 * @param neighbor
 * @param targetBrick
 * @param targetNeighbor
 */
void
Brick::initNeighbor(const uint8_t side,
                    uint32_t targetBrickId)
{
    neighbors[side] = targetBrickId;
}
