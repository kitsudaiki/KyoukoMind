/**
 * @file        brick.h
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

#ifndef BRICK_H
#define BRICK_H

#include <common.h>
#include <kyouko_root.h>

#include <core/objects/edges.h>

class Brick
{

public:
    //----------------------------------------------------------------------------------------------
    struct PossibleNext
    {
        uint8_t next[5];
    } __attribute__((packed));
    //----------------------------------------------------------------------------------------------
    struct BrickPos
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
    } __attribute__((packed));
    //----------------------------------------------------------------------------------------------

    // common
    uint32_t brickId = UNINIT_STATE_32;
    uint32_t nodeBrickId = UNINIT_STATE_32;
    bool isOutputBrick = false;
    bool isInputBrick = false;

    BrickPos brickPos;

    // 0 - 21: neighbor-bricks
    // 22: the current brick
    uint32_t neighbors[12];
    uint32_t nodePos = UNINIT_STATE_32;

    uint32_t nodeActivity = 0;
    uint32_t synapseActivity = 0;
    uint32_t edgeCreateActivity = 0;
    uint32_t edgeDeleteActivity = 0;
    uint32_t synapseCreateActivity = 0;
    uint32_t synapseDeleteActivity = 0;

    //----------------------------------------------------------------------------------------------

    Brick();
    Brick(const uint32_t &brickId,
          const uint32_t x,
          const uint32_t y);

    Brick(const Brick &other);
    Brick& operator=(const Brick &other);

    ~Brick();

    uint32_t getRandomNeighbor(const uint32_t location, const bool random = false);

    bool connectBricks(const uint8_t sourceSide,
                       Brick &targetBrick);
    bool disconnectBricks(const uint8_t sourceSide);

    uint32_t registerInput();
    void setInputValue(const uint32_t pos, const float value);
    const std::vector<float> getInputValues();
    void clearInput();

    uint32_t registerOutput();
    void setOutputValue(const uint32_t pos, const float value);
    void resetOutputValues();
    uint32_t getNumberOfOutputValues();
    const std::vector<float> getOutputValues();

    void setShouldValue(const uint32_t pos, const float value);
    const std::vector<float> getShouldValues();

private:
    void initNeighborList();

    void initNeighbor(const uint8_t side,
                      uint32_t targetBrickId);
    bool uninitNeighbor(const uint8_t side);

    const PossibleNext getPossibleNext(const uint8_t inputSide, const bool random = false);
    void updateMonitoringMetadata();


    std::vector<float> m_inputs;
    std::atomic_flag m_input_lock = ATOMIC_FLAG_INIT;

    std::vector<float> m_outputs;
    std::atomic_flag m_output_lock = ATOMIC_FLAG_INIT;

    std::vector<float> m_should;
    std::atomic_flag m_should_lock = ATOMIC_FLAG_INIT;
};

#endif // BRICK_H
