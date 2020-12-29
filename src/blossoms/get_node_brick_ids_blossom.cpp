/**
 * @file        get_node_brick_ids_blossom.cpp
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

#include "get_node_brick_ids_blossom.h"

#include <libKitsunemimiPersistence/logger/logger.h>

#include <core/objects/segment.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

GetNodeBrickIds_Blossom::GetNodeBrickIds_Blossom()
{
    validationMap.emplace("node_brick_ids", BlossomValidDef(IO_ValueType::OUTPUT_TYPE, true));
}

bool
GetNodeBrickIds_Blossom::runTask(BlossomLeaf &blossomLeaf,
                                 std::string &)
{
    LOG_DEBUG("get node brick ids");

    Brick* brick = getBuffer<Brick>(KyoukoRoot::m_segment->bricks);

    DataArray* result = new DataArray();
    for(uint64_t i = 0; i < KyoukoRoot::m_segment->bricks.numberOfItems; i++)
    {
        const uint32_t id = brick[i].nodeBrickId;
        if(id != UNINIT_STATE_32) {
            result->append(new DataValue(static_cast<long>(brick[i].brickId)));
        }
    }

    blossomLeaf.output.insert("node_brick_ids", result);

    return true;
}
