/**
 * @file        cluster_table.cpp
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

#include "cluster_table.h"

#include <libKitsunemimiCommon/common_items/table_item.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiSakuraDatabase/sql_database.h>

/**
 * @brief constructor
 */
ClusterTable::ClusterTable(Kitsunemimi::Sakura::SqlDatabase* db)
    : HanamiSqlTable(db)
{
    m_tableName = "clusters";

    DbHeaderEntry clusterName;
    clusterName.name = "cluster_name";
    clusterName.maxLength = 256;
    m_tableHeader.push_back(clusterName);

    DbHeaderEntry clusterUuid;
    clusterUuid.name = "internal_cluster_uuid";
    clusterUuid.maxLength = 36;
    m_tableHeader.push_back(clusterUuid);

    DbHeaderEntry templateString;
    templateString.name = "template";
    m_tableHeader.push_back(templateString);
}

ClusterTable::~ClusterTable() {}

/**
 * @brief add a new cluster to the database
 *
 * @param userData json-item with all information of the cluster to add to database
 * @param error reference for error-output
 *
 * @return true, if successfull, else false
 */
bool
ClusterTable::addCluster(Kitsunemimi::Json::JsonItem &clusterData,
                         Kitsunemimi::ErrorContainer &error)
{
    return add(clusterData, error);
}

/**
 * @brief get a cluster from the database by his name
 *
 * @param result reference for the result-output in case that a cluster with this name was found
 * @param userName name of the requested cluster
 * @param error reference for error-output
 * @param showHiddenValues set to true to also show as hidden marked fields
 *
 * @return true, if successfull, else false
 */
bool
ClusterTable::getClusterByName(Kitsunemimi::Json::JsonItem &result,
                               const std::string &clusterName,
                               Kitsunemimi::ErrorContainer &error,
                               const bool showHiddenValues)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("cluster_name", clusterName);

    // get user from db
    if(get(result, conditions, error, showHiddenValues) == false)
    {
        LOG_ERROR(error);
        return false;
    }

    return true;
}

/**
 * @brief get all clusters from the database table
 *
 * @param result reference for the result-output
 * @param error reference for error-output
 *
 * @return true, if successfull, else false
 */
bool
ClusterTable::getAllCluster(Kitsunemimi::TableItem &result,
                            Kitsunemimi::ErrorContainer &error)
{
    return getAll(result, error);
}

/**
 * @brief delete a cluster from the table
 *
 * @param userName name of the cluster to delete
 * @param error reference for error-output
 *
 * @return true, if successfull, else false
 */
bool
ClusterTable::deleteCluster(const std::string &clusterName,
                            Kitsunemimi::ErrorContainer &error)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("cluster_name", clusterName);

    return deleteFromDb(conditions, error);
}
