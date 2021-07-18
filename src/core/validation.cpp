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

#include <core/objects/segment.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/network_cluster.h>

void
validateStructSizes()
{
    assert(sizeof(Brick) == 4096);

    assert(sizeof(Synapse) == 8);

    assert(sizeof(SynapseSection) == 512);
    assert(sizeof(SegmentHeader) == 256);
    assert(sizeof(Brick) == 4096);
    assert(sizeof(Node) == 32);

    assert(sizeof(NetworkMetaData) == 256);
    assert(sizeof(InitSettings) == 256);
    assert(sizeof(SegmentSettings) == 256);

    return;
}
