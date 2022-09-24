/**
 * @file        upload_template.cpp
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

#include "upload_template.h"
#include <kyouko_root.h>

#include <libShioriArchive/datasets.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;

UploadTemplate::UploadTemplate()
    : Blossom("Upload a new template and store it within the database.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("name",
                       SAKURA_STRING_TYPE,
                       true,
                       "Name for the new template.");
    // column in database is limited to 256 characters size
    assert(addFieldBorder("name", 4, 256));
    assert(addFieldRegex("name", "[a-zA-Z][a-zA-Z_0-9]*"));

    registerInputField("template",
                       SAKURA_MAP_TYPE,
                       true,
                       "New template to upload.");

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new uploaded template.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new uploaded template.");
    registerOutputField("owner_id",
                        SAKURA_STRING_TYPE,
                        "ID of the user, who created the new template.");
    registerOutputField("project_id",
                        SAKURA_STRING_TYPE,
                        "ID of the project, where the new template belongs to.");
    registerOutputField("visibility",
                        SAKURA_STRING_TYPE,
                        "Visibility of the new created template (private, shared, public).");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
UploadTemplate::runTask(BlossomLeaf &blossomLeaf,
                        const Kitsunemimi::DataMap &context,
                        BlossomStatus &status,
                        Kitsunemimi::ErrorContainer &error)
{
    const std::string name = blossomLeaf.input.get("name").getString();
    const std::string stringContent = blossomLeaf.input.get("template").toString();
    const Kitsunemimi::Hanami::UserContext userContext(context);

    // check if template with the name already exist within the table
    Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::templateTable->getTemplateByName(getResult, name, userContext, error))
    {
        status.errorMessage = "Template with name '" + name + "' already exist.";
        status.statusCode = Kitsunemimi::Hanami::CONFLICT_RTYPE;
        error.addMeesage(status.errorMessage);
        return false;
    }

    // convert template to base64 to be storage into database
    std::string base64Content;
    Kitsunemimi::Crypto::encodeBase64(base64Content, stringContent.c_str(), stringContent.size());

    // convert values
    Kitsunemimi::Json::JsonItem templateData;
    templateData.insert("name", name);
    templateData.insert("data", base64Content);
    templateData.insert("visibility", "private");

    // add new user to table
    if(KyoukoRoot::templateTable->addTemplate(templateData, userContext, error) == false)
    {
        error.addMeesage("Failed to add new template to database");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // get new created user from database
    if(KyoukoRoot::templateTable->getTemplateByName(blossomLeaf.output,
                                                    name,
                                                    userContext,
                                                    error) == false)
    {
        error.addMeesage("Failed to get new template from database");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    return true;
}
