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
    // input
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

    // output
    registerOutputField("uuid",
                        SAKURA_STRING_TYPE,
                        "UUID of the new created template.");
    registerOutputField("name",
                        SAKURA_STRING_TYPE,
                        "Name of the new created template.");
}

bool
CreateTemplate::runTask(BlossomLeaf &blossomLeaf,
                        const Kitsunemimi::DataMap &,
                        BlossomStatus &status,
                        Kitsunemimi::ErrorContainer &error)
{
    const std::string name = blossomLeaf.input.get("name").getString();
    const long numberOfInputs = blossomLeaf.input.get("number_of_inputs").getLong();
    const long numberOfOutputs = blossomLeaf.input.get("number_of_outputs").getLong();

    // check if user already exist within the table
    Kitsunemimi::Json::JsonItem getResult;
    if(KyoukoRoot::templateTable->getTemplateByName(getResult, name, error))
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
    if(KyoukoRoot::templateTable->addTemplate(templateData, error) == false)
    {
        status.statusCode = Kitsunemimi::Hanami::INTERNAL_SERVER_ERROR_RTYPE;
        return false;
    }

    // get new created user from database
    if(KyoukoRoot::templateTable->getTemplateByName(blossomLeaf.output, name, error) == false)
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
 * @param name
 * @param numberOfInputNodes
 * @param numberOfOutputNodes
 * @return
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
 * @param result
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
 * @param result
 * @param numberOfInputNodes
 * @param numberOfOutputNodes
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
 * @param resultconst
 * @param numberOfInputNodes
 * @return
 */
uint32_t
CreateTemplate::createInputSegments(DataArray* result,
                                    const long numberOfInputNodes)
{
    DataMap* newSegment = new DataMap();

    DataArray* position = new DataArray();
    position->append(new DataValue(1));
    position->append(new DataValue(1));
    position->append(new DataValue(1));

    newSegment->insert("type", new DataValue("input_segment"));
    newSegment->insert("position", position);
    newSegment->insert("number_of_inputs", new DataValue(numberOfInputNodes));

    result->append(newSegment);

    return 1;
}

/**
 * @brief CreateClusterGenerate::createDynamicSegments
 * @param result
 * @param numberOfInputNodes
 * @param numberOfOutputNodes
 */
void
CreateTemplate::createDynamicSegments(DataArray* result,
                                      const long numberOfInputNodes,
                                      const long numberOfOutputNodes)
{
    DataMap* newSegment = new DataMap();

    DataArray* position = new DataArray();
    position->append(new DataValue(1));
    position->append(new DataValue(2));
    position->append(new DataValue(1));

    newSegment->insert("type", new DataValue("dynamic_segment"));
    newSegment->insert("position", position);

    createSegmentSettings(newSegment);
    createSegmentBricks(newSegment, numberOfInputNodes, numberOfOutputNodes);

    result->append(newSegment);
}

/**
 * @brief CreateClusterGenerate::createSegmentSettings
 * @param result
 */
void
CreateTemplate::createSegmentSettings(DataMap* result)
{
    DataMap* settings = new DataMap();

    settings->insert("refraction_time", new DataValue(1));
    settings->insert("synapse_delete_border", new DataValue(0.01));
    settings->insert("action_potential", new DataValue(0.0));
    settings->insert("node_cooldown", new DataValue(3000.0));
    settings->insert("memorizing", new DataValue(0.5));
    settings->insert("glia_value", new DataValue(1.0));
    settings->insert("max_synapse_weight", new DataValue(0.05));
    settings->insert("sign_neg", new DataValue(0.5));
    settings->insert("potential_overflow", new DataValue(1.0));
    settings->insert("multiplicator_range", new DataValue(1));
    settings->insert("node_lower_border", new DataValue(2.0));
    settings->insert("node_upper_border", new DataValue(7.0));
    settings->insert("max_synapse_sections", new DataValue(2500));

    result->insert("settings", settings);
}

/**
 * @brief CreateClusterGenerate::createSegmentBricks
 * @param result
 * @param numberOfInputNodes
 * @param numberOfOutputNodes
 */
void
CreateTemplate::createSegmentBricks(DataMap* result,
                                    const long numberOfInputNodes,
                                    const long numberOfOutputNodes)
{
    DataArray* bricks = new DataArray();

    DataMap* inputBrick = new DataMap();
    DataArray* inputBrickPosition = new DataArray();
    inputBrickPosition->append(new DataValue(1));
    inputBrickPosition->append(new DataValue(1));
    inputBrickPosition->append(new DataValue(1));
    inputBrick->insert("type", new DataValue("input"));
    inputBrick->insert("number_of_nodes", new DataValue(numberOfInputNodes));
    inputBrick->insert("position", inputBrickPosition);
    bricks->append(inputBrick);

    DataMap* internalBrick = new DataMap();
    DataArray* internalBrickPosition = new DataArray();
    internalBrickPosition->append(new DataValue(2));
    internalBrickPosition->append(new DataValue(1));
    internalBrickPosition->append(new DataValue(1));
    internalBrick->insert("type", new DataValue("normal"));
    internalBrick->insert("number_of_nodes", new DataValue(300));
    internalBrick->insert("position", internalBrickPosition);
    bricks->append(internalBrick);

    DataMap* outputBrick = new DataMap();
    DataArray* outputBrickPosition = new DataArray();
    outputBrickPosition->append(new DataValue(3));
    outputBrickPosition->append(new DataValue(1));
    outputBrickPosition->append(new DataValue(1));
    outputBrick->insert("type", new DataValue("output"));
    outputBrick->insert("number_of_nodes", new DataValue(numberOfOutputNodes));
    outputBrick->insert("position", outputBrickPosition);
    bricks->append(outputBrick);

    result->insert("bricks", bricks);
}

/**
 * @brief CreateClusterGenerate::createOutputSegments
 * @param resultconst
 * @param numberOfOutputNodes
 * @return
 */
uint32_t
CreateTemplate::createOutputSegments(DataArray* result,
                                     const long numberOfOutputNodes)
{
    DataMap* newSegment = new DataMap();

    DataArray* position = new DataArray();
    position->append(new DataValue(1));
    position->append(new DataValue(3));
    position->append(new DataValue(1));

    newSegment->insert("type", new DataValue("output_segment"));
    newSegment->insert("position", position);
    newSegment->insert("number_of_outputs", new DataValue(numberOfOutputNodes));

    result->append(newSegment);

    return 1;
}
