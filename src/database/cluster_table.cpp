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
    clusterName.name = "name";
    clusterName.maxLength = 256;
    m_tableHeader.push_back(clusterName);

    DbHeaderEntry templateUuid;
    templateUuid.name = "template_uuid";
    templateUuid.maxLength = 36;
    m_tableHeader.push_back(templateUuid);
}

ClusterTable::~ClusterTable() {}

/**
 * @brief add a new cluster to the database
 *
 * @param userData json-item with all information of the cluster to add to database
 * @param userUuid user-uuid to filter
 * @param projectUuid project-uuid to filter
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
ClusterTable::addCluster(Kitsunemimi::Json::JsonItem &clusterData,
                         const std::string &userUuid,
                         const std::string &projectUuid,
                         Kitsunemimi::ErrorContainer &error)
{
    return add(clusterData, userUuid, projectUuid, error);
}

/**
 * @brief get a cluster from the database by his name
 *
 * @param result reference for the result-output in case that a cluster with this name was found
 * @param clusterName name of the requested cluster
 * @param userUuid user-uuid to filter
 * @param projectUuid project-uuid to filter
 * @param isAdmin true, if use who makes request is admin
 * @param error reference for error-output
 * @param showHiddenValues set to true to also show as hidden marked fields
 *
 * @return true, if successful, else false
 */
bool
ClusterTable::getClusterByName(Kitsunemimi::Json::JsonItem &result,
                               const std::string &clusterName,
                               const std::string &userUuid,
                               const std::string &projectUuid,
                               const bool isAdmin,
                               Kitsunemimi::ErrorContainer &error,
                               const bool showHiddenValues)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("name", clusterName);

    // get user from db
    if(get(result, userUuid, projectUuid, isAdmin, conditions, error, showHiddenValues) == false) {
        return false;
    }

    return true;
}

/**
 * @brief get all clusters from the database table
 *
 * @param result reference for the result-output
 * @param userUuid user-uuid to filter
 * @param projectUuid project-uuid to filter
 * @param isAdmin true, if use who makes request is admin
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
ClusterTable::getAllCluster(Kitsunemimi::TableItem &result,
                            const std::string &userUuid,
                            const std::string &projectUuid,
                            const bool isAdmin,
                            Kitsunemimi::ErrorContainer &error)
{
    return getAll(result, userUuid, projectUuid, isAdmin, error);
}

/**
 * @brief delete a cluster from the table
 *
 * @param clusterName name of the cluster to delete
 * @param userUuid user-uuid to filter
 * @param projectUuid project-uuid to filter
 * @param isAdmin true, if use who makes request is admin
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
ClusterTable::deleteCluster(const std::string &clusterName,
                            const std::string &userUuid,
                            const std::string &projectUuid,
                            const bool isAdmin,
                            Kitsunemimi::ErrorContainer &error)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("name", clusterName);

    return del(conditions, userUuid, projectUuid, isAdmin, error);
}
