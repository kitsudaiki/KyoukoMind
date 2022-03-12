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

/**
 * @brief generate a new template
 *
 * @param name name of the cluster
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 *
 * @return data-map with the new cluster-template
 */
DataMap*
CreateTemplate::generateNewTemplate(const std::string name,
                                    const long numberOfInputNodes,
                                    const long numberOfOutputNodes,
                                    const JsonItem &settingsOverride)
{
    DataMap* result = new DataMap();
    result->insert("name", new DataValue(name));

    createClusterSettings(result);
    createSegments(result, numberOfInputNodes, numberOfOutputNodes, settingsOverride);

    return result;
}

/**
 * @brief create seggings-block
 *
 * @param result pointer to the resulting object
 */
void
CreateTemplate::createClusterSettings(DataMap* result)
{
    DataMap* settings = new DataMap();

    settings->insert("cycle_time", new DataValue(1000000));
    settings->insert("max_brick_distance", new DataValue(10));

    result->insert("settings", settings);
}

/**
 * @brief create segments
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 */
void
CreateTemplate::createSegments(DataMap* result,
                               const long numberOfInputNodes,
                               const long numberOfOutputNodes,
                               const JsonItem &settingsOverride)
{
    DataArray* segments = new DataArray();
    result->insert("segments", segments);

    createInputSegments(segments, numberOfInputNodes);
    createDynamicSegments(segments, numberOfInputNodes, numberOfOutputNodes, settingsOverride);
    createOutputSegments(segments, numberOfOutputNodes);
}

/**
 * @brief create input-segments
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 *
 * @return number of new input-segments
 */
uint32_t
CreateTemplate::createInputSegments(DataArray* result,
                                    const long numberOfInputNodes)
{
    DataMap* newSegment = new DataMap();

    newSegment->insert("type", new DataValue("input_segment"));
    newSegment->insert("position", createPosition(1, 1, 1));
    newSegment->insert("number_of_inputs", new DataValue(numberOfInputNodes));

    result->append(newSegment);

    return 1;
}

/**
 * @brief create internal dynamic segments
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 */
void
CreateTemplate::createDynamicSegments(DataArray* result,
                                      const long numberOfInputNodes,
                                      const long numberOfOutputNodes,
                                      const JsonItem &settingsOverride)
{
    DataMap* newSegment = new DataMap();

    //newSegment->insert("type", new DataValue("static_segment"));
    newSegment->insert("type", new DataValue("dynamic_segment"));
    newSegment->insert("position", createPosition(1, 2, 1));

    createSegmentSettings(newSegment, settingsOverride);
    createSegmentBricks(newSegment, numberOfInputNodes, numberOfOutputNodes);

    result->append(newSegment);
}

/**
 * @brief create settings block for the segment
 *
 * @param result pointer to the resulting object
 */
void
CreateTemplate::createSegmentSettings(DataMap* result,
                                      const JsonItem &settingsOverride)
{
    DataMap* settings = new DataMap();

    settings->insert("refraction_time", new DataValue(1));
    settings->insert("synapse_delete_border", new DataValue(0.005));
    settings->insert("action_potential", new DataValue(5.0));
    settings->insert("node_cooldown", new DataValue(3000.0));
    settings->insert("memorizing", new DataValue(0.5));
    settings->insert("glia_value", new DataValue(1.1));
    settings->insert("max_synapse_weight", new DataValue(0.025));
    settings->insert("sign_neg", new DataValue(0.5));
    settings->insert("potential_overflow", new DataValue(1.0));
    settings->insert("multiplicator_range", new DataValue(1));
    settings->insert("max_synapse_sections", new DataValue(5000));

    const std::vector<std::string> keys = settingsOverride.getKeys();
    for(const std::string &key : keys) {
        settings->insert(key, settingsOverride.get(key).getItemContent()->copy(), true);
    }

    result->insert("settings", settings);
}

/**
 * @brief create bricks for an internal segment
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 */
void
CreateTemplate::createSegmentBricks(DataMap* result,
                                    const long numberOfInputNodes,
                                    const long numberOfOutputNodes)
{
    DataArray* bricks = new DataArray();

    // input-part
    DataMap* inputBrick = new DataMap();
    inputBrick->insert("type", new DataValue("input"));
    inputBrick->insert("number_of_nodes", new DataValue(numberOfInputNodes));
    inputBrick->insert("position", createPosition(1, 1, 1));
    bricks->append(inputBrick);

    // centre part
    DataMap* internalBrick = new DataMap();
    internalBrick->insert("type", new DataValue("normal"));
    internalBrick->insert("number_of_nodes", new DataValue(300));
    internalBrick->insert("position", createPosition(2, 1, 1));
    bricks->append(internalBrick);

    // output-part
    DataMap* outputBrick = new DataMap();
    outputBrick->insert("type", new DataValue("output"));
    outputBrick->insert("number_of_nodes", new DataValue(numberOfOutputNodes));
    outputBrick->insert("position", createPosition(3, 1, 1));
    bricks->append(outputBrick);

    result->insert("bricks", bricks);
}

/**
 * @brief create new output-segment
 *
 * @param result pointer to the resulting object
 * @param numberOfOutputNodes number of output-nodes of the cluster
 *
 * @return number of new output-segments
 */
uint32_t
CreateTemplate::createOutputSegments(DataArray* result,
                                     const long numberOfOutputNodes)
{
    DataMap* newSegment = new DataMap();

    newSegment->insert("type", new DataValue("output_segment"));
    newSegment->insert("position", createPosition(1, 3, 1));
    newSegment->insert("number_of_outputs", new DataValue(numberOfOutputNodes));

    result->append(newSegment);

    return 1;
}

/**
 * @brief convert position-information into data-array
 *
 * @param x x-position
 * @param y y-position
 * @param z z-position
 *
 * @return data-array with position-values
 */
DataArray*
CreateTemplate::createPosition(const long x,
                               const long y,
                               const long z)
{
    DataArray* position = new DataArray();

    position->append(new DataValue(x));
    position->append(new DataValue(y));
    position->append(new DataValue(z));

    return position;
}
