/**
 * @file        validation.cpp
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

#include "struct_validation.h"

#include <core/segments/abstract_segment.h>
#include <core/cluster/cluster.h>
#include <core/segments/brick.h>

#include <core/segments/dynamic_segment/objects.h>

/**
 * @brief validate to sized of all structs to ensure, that they have all the defined size of 2^x
 */
void
validateStructSizes()
{
    assert(sizeof(Synapse) == 8);
    assert(sizeof(SynapseSection) == 512);
    assert(sizeof(SegmentHeader) == 512);
    assert(sizeof(SegmentName) == 256);
    assert(sizeof(Brick) == 4096);
    assert(sizeof(DynamicNode) == 32);

    assert(sizeof(ClusterMetaData) == 2048);
    assert(sizeof(ClusterSettings) == 256);
    assert(sizeof(DynamicSegmentSettings) == 256);
    assert(sizeof(Kitsunemimi::Hanami::kuuid) == 40);

    return;
}
