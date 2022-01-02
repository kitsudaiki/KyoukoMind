/**
 * @file        list_cluster.cpp
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

#include "list_cluster.h"

#include <kyouko_root.h>

#include <libKitsunemimiHanamiCommon/enums.h>

using namespace Kitsunemimi::Sakura;

ListCluster::ListCluster()
    : Blossom("List all clusters.")
{
    // output
    registerOutputField("header",
                        SAKURA_ARRAY_TYPE,
                        "Array with the namings all columns of the table.");
    assert(addFieldMatch("header", new Kitsunemimi::DataValue("[\"uuid\","
                                                              "\"name\","
                                                              "\"template_uuid\"]")));
    registerOutputField("body",
                        SAKURA_ARRAY_TYPE,
                        "Array with all rows of the table, which array arrays too.");
}

bool
ListCluster::runTask(BlossomLeaf &blossomLeaf,
                     const Kitsunemimi::DataMap &,
                     BlossomStatus &status,
                     Kitsunemimi::ErrorContainer &error)
{
    // get data from table
    Kitsunemimi::TableItem table;
    if(KyoukoRoot::clustersTable->getAllCluster(table, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    table.deleteColumn("visibility");
    table.deleteColumn("owner_uuid");
    table.deleteColumn("project_uuid");

    blossomLeaf.output.insert("header", table.getInnerHeader());
    blossomLeaf.output.insert("body", table.getBody());

    return true;
}