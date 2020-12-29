/**
 * @file        validation.h
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

#include "validation.h"

#include <core/objects/item_buffer.h>
#include <core/objects/segment.h>

#include <core/objects/global_values.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/edges.h>

void
validateStructSizes()
{
    assert(sizeof(Brick) < 4096);

    assert(sizeof(Synapse) == 16);

    std::cout<<"sizeof(SynapseSection): "<<sizeof(SynapseSection)<<std::endl;
    assert(sizeof(SynapseSection) == 256);

    std::cout<<"sizeof(GlobalValues): "<<sizeof(GlobalValues)<<std::endl;
    assert(sizeof(GlobalValues) == 256);

    assert(sizeof(EdgeSection) == 4096);

    return;
}
