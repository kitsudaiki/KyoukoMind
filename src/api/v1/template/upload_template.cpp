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

#include <libSagiriArchive/sagiri_send.h>

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
    const JsonItem settingsOverride = blossomLeaf.input.get("template");

    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");
    const std::string token = context.getStringByKey("token");

    // check if template with the name already exist within the table
    Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::templateTable->getTemplateByName(getResult,
                                                    name,
                                                    userUuid,
                                                    projectUuid,
                                                    isAdmin,
                                                    error))
    {
        status.errorMessage = "Template with name '" + name + "' already exist.";
        status.statusCode = Kitsunemimi::Hanami::CONFLICT_RTYPE;
        return false;
    }

    const std::string stringContent = settingsOverride.toString();
    // std::cout<<generatedContent->toString(true)<<std::endl;

    // convert template to base64 to be storage into database
    std::string base64Content;
    Kitsunemimi::Crypto::encodeBase64(base64Content, stringContent.c_str(), stringContent.size());

    // convert values
    Kitsunemimi::Json::JsonItem templateData;
    templateData.insert("name", name);
    templateData.insert("data", base64Content);
    // TODO: fill project- and owner-id
    templateData.insert("project_uuid", "-");
    templateData.insert("owner_uuid", "-");
    templateData.insert("visibility", 0);

    // add new user to table
    if(KyoukoRoot::templateTable->addTemplate(templateData,
                                              userUuid,
                                              projectUuid,
                                              error) == false)
    {
        error.addMeesage("Failed to add new template to database");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // get new created user from database
    if(KyoukoRoot::templateTable->getTemplateByName(blossomLeaf.output,
                                                    name,
                                                    userUuid,
                                                    projectUuid,
                                                    isAdmin,
                                                    error) == false)
    {
        error.addMeesage("Failed to get new template from database");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_uuid");
    blossomLeaf.output.remove("project_uuid");
    blossomLeaf.output.remove("visibility");

    return true;
}
