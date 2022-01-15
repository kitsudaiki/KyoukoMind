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

#include <libKitsunemimiHanamiCommon/uuid.h>
#include <libKitsunemimiHanamiCommon/enums.h>

#include <libKitsunemimiCrypto/common.h>

#include <kyouko_root.h>

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

    registerInputField("number_of_inputs",
                       SAKURA_INT_TYPE,
                       true,
                       "Number of input-nodes of the new template.");
    assert(addFieldBorder("number_of_inputs", 1, 100000000));

    registerInputField("number_of_outputs",
                       SAKURA_INT_TYPE,
                       true,
                       "Number of output-nodes of the new template.");
    assert(addFieldBorder("number_of_outputs", 1, 100000000));

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
    const long numberOfInputs = blossomLeaf.input.get("number_of_inputs").getLong();
    const long numberOfOutputs = blossomLeaf.input.get("number_of_outputs").getLong();
    const std::string userUuid = context.getStringByKey("uuid");
    const std::string projectUuid = context.getStringByKey("projects");
    const bool isAdmin = context.getBoolByKey("is_admin");

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

    DataItem* generatedContent = generateNewCluster(name, numberOfInputs, numberOfOutputs);
    const std::string stringContent = generatedContent->toString();

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
 * @brief CreateClusterGenerate::generateNewCluster
 *
 * @param name name of the cluster
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 *
 * @return data-map with the new cluster-template
 */
DataMap*
CreateTemplate::generateNewCluster(const std::string name,
                                   const long numberOfInputNodes,
                                   const long numberOfOutputNodes)
{
    DataMap* result = new DataMap();
    result->insert("name", new DataValue(name));

    createClusterSettings(result);
    createSegments(result, numberOfInputNodes, numberOfOutputNodes);

    return result;
}

/**
 * @brief CreateClusterGenerate::createClusterSettings
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
 * @brief CreateClusterGenerate::createSegments
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 */
void
CreateTemplate::createSegments(DataMap* result,
                               const long numberOfInputNodes,
                               const long numberOfOutputNodes)
{
    DataArray* segments = new DataArray();
    result->insert("segments", segments);

    createInputSegments(segments, numberOfInputNodes);
    createDynamicSegments(segments, numberOfInputNodes, numberOfOutputNodes);
    createOutputSegments(segments, numberOfOutputNodes);
}

/**
 * @brief CreateClusterGenerate::createInputSegments
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
 * @brief CreateClusterGenerate::createDynamicSegments
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 */
void
CreateTemplate::createDynamicSegments(DataArray* result,
                                      const long numberOfInputNodes,
                                      const long numberOfOutputNodes)
{
    DataMap* newSegment = new DataMap();

    newSegment->insert("type", new DataValue("dynamic_segment"));
    newSegment->insert("position", createPosition(1, 2, 1));

    createSegmentSettings(newSegment);
    createSegmentBricks(newSegment, numberOfInputNodes, numberOfOutputNodes);

    result->append(newSegment);
}

/**
 * @brief create settings block for the segment
 *
 * @param result pointer to the resulting object
 */
void
CreateTemplate::createSegmentSettings(DataMap* result)
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
    settings->insert("node_lower_border", new DataValue(2.0));
    settings->insert("node_upper_border", new DataValue(7.0));
    settings->insert("max_synapse_sections", new DataValue(2500));

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
