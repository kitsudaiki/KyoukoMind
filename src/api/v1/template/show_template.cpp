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

#include "show_template.h"

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCrypto/common.h>

#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

ShowTemplate::ShowTemplate()
    : Blossom("Show a specific template.")
{
    //----------------------------------------------------------------------------------------------
    // input
    //----------------------------------------------------------------------------------------------

    registerInputField("uuid",
                       SAKURA_STRING_TYPE,
                       true,
                       "UUID of the template.");
    assert(addFieldRegex("uuid", "[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-"
                                 "[a-fA-F0-9]{4}-[a-fA-F0-9]{12}"));

    //----------------------------------------------------------------------------------------------
    // output
    //----------------------------------------------------------------------------------------------

    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the template.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the template.");
    registerOutputField("template",
                        SAKURA_MAP_TYPE,
                        "The template itself.");
    registerOutputField("owner_uuid",
                        SAKURA_STRING_TYPE,
                        "Owner of the template.");
    registerOutputField("project_uuid",
                        SAKURA_STRING_TYPE,
                        "Project of the template.");
    registerOutputField("visibility",
                        SAKURA_STRING_TYPE,
                        "Visibility of the template.");

    //----------------------------------------------------------------------------------------------
    //
    //----------------------------------------------------------------------------------------------
}

/**
 * @brief runTask
 */
bool
ShowTemplate::runTask(BlossomLeaf &blossomLeaf,
                      const Kitsunemimi::DataMap &context,
                      BlossomStatus &status,
                      Kitsunemimi::ErrorContainer &error)
{
    // get information from request
    const std::string uuid = blossomLeaf.input.get("uuid").getString();
    // TODO: check type-field

    // get context-info
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

    // get data from table
    if(KyoukoRoot::templateTable->getTemplate(blossomLeaf.output,
                                              uuid,
                                              userUuid,
                                              projectUuid,
                                              isAdmin,
                                              error,
                                              true) == false)
    {
        status.errorMessage = "Tempalte with UUID '" + uuid + "' not found.";
        status.statusCode = Kitsunemimi::Hanami::NOT_FOUND_RTYPE;
        error.addMeesage(status.errorMessage);
        return false;
    }

    // decode template
    std::string decodedTemplate;
    if(Kitsunemimi::Crypto::decodeBase64(decodedTemplate,
                                         blossomLeaf.output.get("data").getString()) == false)
    {
        error.addMeesage("base64-decoding of the template failed");
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // parse template
    JsonItem parsedTemplate;
    if(parsedTemplate.parse(decodedTemplate, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        error.addMeesage("Failed to parse decoded template");
        return false;
    }
    blossomLeaf.output.insert("template", parsedTemplate.getItemContent()->copy());

    // remove irrelevant fields
    blossomLeaf.output.remove("data");

    return true;
}
