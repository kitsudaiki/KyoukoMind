/**
 * @file        create_cluster_template.h
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

#include "create_template.h"
#include <kyouko_root.h>
#include <core/templates/template_creator.h>

#include <libSagiriArchive/sagiri_send.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;

CreateTemplate::CreateTemplate()
    : Blossom("Create new template-file and store it within the database.")
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

    registerInputField("data_set_uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the data-set to request number of inputs and outputs.");
    assert(addFieldRegex("data_set_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                          "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    registerInputField("settings_override",
                       SAKURA_MAP_TYPE,
                       false,
                       "Map with override information for the settings-block of the new template.");

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created template.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new created template.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
CreateTemplate::runTask(BlossomLeaf &blossomLeaf,
                        const Kitsunemimi::DataMap &context,
                        BlossomStatus &status,
                        Kitsunemimi::ErrorContainer &error)
{
    const std::string name = blossomLeaf.input.get("name").getString();
    const std::string dataSetUuid = blossomLeaf.input.get("data_set_uuid").getString();
    const JsonItem settingsOverride = blossomLeaf.input.get("settings_override");

    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");
    const std::string token = context.getStringByKey("token");

    // check if user already exist within the table
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

    // get meta-infos of data-set from sagiri
    Kitsunemimi::Json::JsonItem dataSetInfo;
    if(Sagiri::getDataSetInformation(dataSetInfo, dataSetUuid, token, error) == false)
    {
        error.addMeesage("failed to get information from sagiri for uuid '" + dataSetUuid + "'");
        // TODO: add status-error from response from sagiri
        status.statusCode = Kitsunemimi::Hanami::UNAUTHORIZED_RTYPE;
        return false;
    }

    // get relevant information from output
    const uint64_t numberOfInputs = dataSetInfo.get("inputs").getLong();
    const uint64_t numberOfOutputs = dataSetInfo.get("outputs").getLong();

    //  generate new template
    DataItem* generatedContent = generateNewTemplate(name,
                                                     numberOfInputs,
                                                     numberOfOutputs,
                                                     //10000,
                                                     //2,
                                                     settingsOverride);
    const std::string stringContent = generatedContent->toString();
    std::cout<<generatedContent->toString(true)<<std::endl;

    // convert template to base64 to be storage into database
    std::string base64Content;
    Kitsunemimi::Crypto::encodeBase64(base64Content, stringContent.c_str(), stringContent.size());

    // convert values
    Kitsunemimi::Json::JsonItem templateData;
    templateData.insert("name", name);
    templateData.insert("data", base64Content);
    templateData.insert("project_uuid", "-");
    templateData.insert("owner_uuid", "-");
    templateData.insert("visibility", 0);

    // add new user to table
    if(KyoukoRoot::templateTable->addTemplate(templateData,
                                              userUuid,
                                              projectUuid,
                                              error) == false)
    {
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
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_uuid");
    blossomLeaf.output.remove("project_uuid");
    blossomLeaf.output.remove("visibility");

    return true;
}
