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

#include "template_table.h"

#include <libKitsunemimiCommon/common_items/table_item.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiSakuraDatabase/sql_database.h>

/**
 * @brief constructor
 */
TemplateTable::TemplateTable(Kitsunemimi::Sakura::SqlDatabase* db)
    : HanamiSqlTable(db)
{
    m_tableName = "templates";

    DbHeaderEntry templateName;
    templateName.name = "name";
    templateName.maxLength = 256;
    m_tableHeader.push_back(templateName);

    DbHeaderEntry templateType;
    templateType.name = "type";
    templateType.maxLength = 10;
    m_tableHeader.push_back(templateType);

    DbHeaderEntry templateString;
    templateString.name = "data";
    templateString.hide = true;
    m_tableHeader.push_back(templateString);
}

/**
 * @brief destructor
 */
TemplateTable::~TemplateTable() {}

/**
 * @brief add a new template to the database
 *
 * @param userData json-item with all information of the cluster to add to database
 * @param userUuid user-uuid to filter
 * @param projectUuid project-uuid to filter
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
TemplateTable::addTemplate(Kitsunemimi::Json::JsonItem &clusterData,
                           const std::string &userUuid,
                           const std::string &projectUuid,
                           Kitsunemimi::ErrorContainer &error)
{
    if(add(clusterData, userUuid, projectUuid, error) == false)
    {
        error.addMeesage("Failed to add template to database");
        return false;
    }

    return true;
}

/**
 * @brief get a template from the database by his name
 *
 * @param result reference for the result-output in case that a cluster with this name was found
 * @param templateUuid uuid of the requested template
 * @param userUuid user-uuid to filter
 * @param projectUuid project-uuid to filter
 * @param isAdmin true, if use who makes request is admin
 * @param error reference for error-output
 * @param showHiddenValues set to true to also show as hidden marked fields
 *
 * @return true, if successful, else false
 */
bool
TemplateTable::getTemplate(Kitsunemimi::Json::JsonItem &result,
                           const std::string &templateUuid,
                           const std::string &type,
                           const std::string &userUuid,
                           const std::string &projectUuid,
                           const bool isAdmin,
                           Kitsunemimi::ErrorContainer &error,
                           const bool showHiddenValues)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("uuid", templateUuid);
    conditions.emplace_back("type", type);

    // get user from db
    if(get(result, userUuid, projectUuid, isAdmin, conditions, error, showHiddenValues) == false)
    {
        error.addMeesage("Failed to get template with UUID '"
                         + templateUuid
                         + "' from database");
        return false;
    }

    return true;
}

/**
 * @brief get a template from the database by his name
 *
 * @param result reference for the result-output in case that a cluster with this name was found
 * @param templateName name of the requested template
 * @param userUuid user-uuid to filter
 * @param projectUuid project-uuid to filter
 * @param isAdmin true, if use who makes request is admin
 * @param error reference for error-output
 * @param showHiddenValues set to true to also show as hidden marked fields
 *
 * @return true, if successful, else false
 */
bool
TemplateTable::getTemplateByName(Kitsunemimi::Json::JsonItem &result,
                                 const std::string &templateName,
                                 const std::string &type,
                                 const std::string &userUuid,
                                 const std::string &projectUuid,
                                 const bool isAdmin,
                                 Kitsunemimi::ErrorContainer &error,
                                 const bool showHiddenValues)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("name", templateName);
    conditions.emplace_back("type", type);

    // get user from db
    if(get(result, userUuid, projectUuid, isAdmin, conditions, error, showHiddenValues) == false)
    {
        error.addMeesage("Failed to get template from database by name '" + templateName + "'");
        return false;
    }

    return true;
}

/**
 * @brief get all templates from the database table
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
TemplateTable::getAllTemplate(Kitsunemimi::TableItem &result,
                              const std::string &type,
                              const std::string &userUuid,
                              const std::string &projectUuid,
                              const bool isAdmin,
                              Kitsunemimi::ErrorContainer &error)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("type", type);

    if(getAll(result, userUuid, projectUuid, isAdmin, conditions, error) == false)
    {
        error.addMeesage("Failed to get all templates from database");
        return false;
    }

    return true;
}

/**
 * @brief delete a cluster from the table
 *
 * @param templateUuid uuid of the template to delete
 * @param userUuid user-uuid to filter
 * @param projectUuid project-uuid to filter
 * @param isAdmin true, if use who makes request is admin
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
TemplateTable::deleteTemplate(const std::string &templateUuid,
                              const std::string &type,
                              const std::string &userUuid,
                              const std::string &projectUuid,
                              const bool isAdmin,
                              Kitsunemimi::ErrorContainer &error)
{
    std::vector<RequestCondition> conditions;
    conditions.emplace_back("uuid", templateUuid);
    conditions.emplace_back("type", type);

    if(del(conditions, userUuid, projectUuid, isAdmin, error) == false)
    {
        error.addMeesage("Failed to delete template with UUID '"
                         + templateUuid
                         + "' from database");
        return false;
    }

    return true;
}
