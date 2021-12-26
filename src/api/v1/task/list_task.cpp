/**
 * @file        list_task.cpp
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

#include "list_task.h"

#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

ListTask::ListTask()
    : Blossom("List all tasks of a cluster.")
{
    registerInputField("cluster_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the cluster, which should process the request");

    // output
    registerOutputField("header",
                        SAKURA_ARRAY_TYPE,
                        "Array with the namings all columns of the table.");
    registerOutputField("body",
                        SAKURA_ARRAY_TYPE,
                        "Array with all rows of the table, which array arrays too.");
}

bool
ListTask::runTask(BlossomLeaf &blossomLeaf,
                  const Kitsunemimi::DataMap &,
                  BlossomStatus &status,
                  Kitsunemimi::ErrorContainer &error)
{

}
