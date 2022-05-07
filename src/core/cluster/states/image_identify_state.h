/**
 * @file        image_identify_state.h
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

#ifndef IMAGEIDENTIFY_STATE_H
#define IMAGEIDENTIFY_STATE_H

#include <libKitsunemimiCommon/threading/event.h>

class Cluster;

class ImageIdentify_State
        : public Kitsunemimi::Event
{
public:
    ImageIdentify_State(Cluster* cluster);
    ~ImageIdentify_State();

    bool processEvent();

private:
    Cluster* m_cluster = nullptr;
};

#endif // IMAGEIDENTIFY_STATE_H
