/**
 * @file        delete_cluster.cpp
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

#include "delete_cluster.h"

using namespace Kitsunemimi::Sakura;

DeleteCluster::DeleteCluster()
    : Blossom("Delete a cluster.")
{
    // input
    registerInputField("cluster_name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the cluster.");
    // column in database is limited to 256 characters size
    assert(addFieldBorder("cluster_name", 4, 256));
    assert(addFieldRegex("cluster_name", "[a-zA-Z][a-zA-Z_0-9]*"));
}

bool
DeleteCluster::runTask(BlossomLeaf &blossomLeaf,
                       const Kitsunemimi::DataMap &,
                       BlossomStatus &status,
                       Kitsunemimi::ErrorContainer &error)
{

}
