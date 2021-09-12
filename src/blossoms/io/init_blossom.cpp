/**
 * @file        init_blossom.cpp
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

#include "init_blossom.h"

#include <libKitsunemimiCommon/buffer/data_buffer.h>
#include <libKitsunemimiJson/json_item.h>

using namespace Kitsunemimi::Sakura;

InitBlossom::InitBlossom()
    : Blossom()
{
    registerField("content", INPUT_TYPE, true);
    registerField("result", OUTPUT_TYPE, true);
}

bool
InitBlossom::runTask(BlossomLeaf &blossomLeaf,
                     std::string &errorMessage)
{
    const std::string content = blossomLeaf.input.getStringByKey("content");

    const bool result = false;
    /*const bool result = initializer.createNewNetwork(content);
    if(result == false) {
        errorMessage = "failed to initialize new network";
    }*/

    blossomLeaf.output.insert("result", new Kitsunemimi::DataValue(result));

    return result;
}
