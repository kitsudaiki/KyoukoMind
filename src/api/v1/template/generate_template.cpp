/**
 * @file        generate_template.cpp
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

#include "generate_template.h"
#include <kyouko_root.h>

#include <libSagiriArchive/datasets.h>

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCrypto/common.h>

using namespace Kitsunemimi::Sakura;

GenerateTemplate::GenerateTemplate()
    : Blossom("Generate a new template-file for a specific dataset "
              "and store it within the database.")
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
                       "UUID of the data-set, which should deliver "
                       "information for the new template.");
    assert(addFieldRegex("data_set_uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                          "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    registerInputField("type",
                       SAKURA_STRING_TYPE,
                       true,
                       "Type of the template to create.");
    //TODO: regex for type

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
GenerateTemplate::runTask(BlossomLeaf &blossomLeaf,
                          const Kitsunemimi::DataMap &context,
                          BlossomStatus &status,
                          Kitsunemimi::ErrorContainer &error)
{
    const std::string name = blossomLeaf.input.get("name").getString();
    const std::string type = blossomLeaf.input.get("type").getString();
    const std::string dataSetUuid = blossomLeaf.input.get("data_set_uuid").getString();
    const JsonItem settingsOverride = blossomLeaf.input.get("settings_override");
    const Kitsunemimi::Hanami::UserContext userContext(context);

    // check if template with the name already exist within the table
    /*Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::templateTable->getTemplateByName(getResult,
                                                    name,
                                                    userId,
                                                    projectId,
                                                    isAdmin,
                                                    error))
    {
        status.errorMessage = "Template with name '" + name + "' already exist.";
        status.statusCode = Kitsunemimi::Hanami::CONFLICT_RTYPE;
        error.addMeesage(status.errorMessage);
        return false;
    }*/

    // get meta-infos of data-set from sagiri
    Kitsunemimi::Json::JsonItem dataSetInfo;
    if(Sagiri::getDataSetInformation(dataSetInfo, dataSetUuid, userContext.token, error) == false)
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
    /*DataItem* generatedContent = generateNewTemplate(name,
                                                     type,
                                                     numberOfInputs,
                                                     numberOfOutputs,
                                                     settingsOverride);
    const std::string stringContent = generatedContent->toString();*/
    const std::string stringContent = "";
    // std::cout<<generatedContent->toString(true)<<std::endl;

    // convert template to base64 to be storage into database
    std::string base64Content;
    Kitsunemimi::Crypto::encodeBase64(base64Content,
                                      stringContent.c_str(),
                                      stringContent.size());

    // convert values
    Kitsunemimi::Json::JsonItem templateData;
    templateData.insert("name", name);
    templateData.insert("data", base64Content);
    // TODO: fill project- and owner-id
    templateData.insert("project_id", "-");
    templateData.insert("owner_id", "-");
    templateData.insert("visibility", "private");

    // add new user to table
    if(KyoukoRoot::templateTable->addTemplate(templateData, userContext, error) == false)
    {
        error.addMeesage("Failed to add new template to database");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // get new created user from database
    /*if(KyoukoRoot::templateTable->getTemplateByName(blossomLeaf.output,
                                                           name,
                                                           userId,
                                                           projectId,
                                                           isAdmin,
                                                           error) == false)
    {
        error.addMeesage("Failed to get new template from database");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }*/

    // remove irrelevant fields
    blossomLeaf.output.remove("owner_id");
    blossomLeaf.output.remove("project_id");
    blossomLeaf.output.remove("visibility");

    return true;
}
