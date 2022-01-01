/**
 * @file        show_cluster.cpp
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

#include "show_cluster.h"

#include <kyouko_root.h>

#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi::Sakura;

ShowCluster::ShowCluster()
    : Blossom("Show information of a specific cluster.")
{
    // input
    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name of the cluster.");
    // column in database is limited to 256 characters size
    assert(addFieldBorder("name", 4, 256));
    assert(addFieldRegex("name", "[a-zA-Z][a-zA-Z_0-9]*"));

    // output
    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the cluster.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the cluster.");
    registerOutputField("template_uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the template, which should be used as base for the cluster.");
}

bool
ShowCluster::runTask(BlossomLeaf &blossomLeaf,
                     const Kitsunemimi::DataMap &,
                     BlossomStatus &status,
                     Kitsunemimi::ErrorContainer &error)
{
    // get information from request
    const std::string clusterName = blossomLeaf.input.get("name").getString();

    // get data from table
    if(KyoukoRoot::clustersTable->getClusterByName(blossomLeaf.output, clusterName, error) == false)
    {
        status.errorMessage = "Cluster with name '" + clusterName + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        return false;
    }

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_uuid");
    blossomLeaf.output.remove("project_uuid");
    blossomLeaf.output.remove("visibility");

    return true;
}
