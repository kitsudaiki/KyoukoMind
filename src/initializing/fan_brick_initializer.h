/**
 * @file        fan_brick_initializer.h
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

#ifndef FANBRICKINITIALIZER_H
#define FANBRICKINITIALIZER_H

#include <common.h>

#include <initializing/brick_initializer.h>

class FanBrickInitializer
        : public BrickInitializer
{
public:
    FanBrickInitializer();

    Brick* getAxonBrick(CoreSegment &segment, Brick* sourceBrick);
    bool initTargetBrickList(CoreSegment &segment, Kitsunemimi::Ai::InitMeataData *initMetaData);

private:
    uint8_t getPossibleNext();
};

#endif // FANBRICKINITIALIZER_H
