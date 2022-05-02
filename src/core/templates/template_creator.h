/**
 * @file        template_creator.h
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

#ifndef TEMPLATECREATOR_H
#define TEMPLATECREATOR_H

#include <common.h>

#include <core/templates/dynamic_segments.h>
#include <core/templates/input_segments.h>
#include <core/templates/output_segments.h>

/**
 * @brief create seggings-block
 *
 * @param result pointer to the resulting object
 */
DataMap*
createClusterSettings()
{
    DataMap* settings = new DataMap();

    settings->insert("cycle_time", new DataValue(1000000));
    settings->insert("max_brick_distance", new DataValue(10));

    return settings;
}

/**
 * @brief create segments
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 */
DataArray*
createSegments(const long numberOfInputNodes,
               const long numberOfOutputNodes,
               const JsonItem &settingsOverride)
{
    DataArray* segments = new DataArray();

    segments->append(createInputSegments(numberOfInputNodes));
    segments->append(createDynamicSegments(numberOfInputNodes,
                                           numberOfOutputNodes,
                                           settingsOverride));
    segments->append(createOutputSegments(numberOfOutputNodes));

    return segments;
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
generateNewTemplate(const std::string name,
                    const long numberOfInputNodes,
                    const long numberOfOutputNodes,
                    const JsonItem &settingsOverride)
{
    DataMap* result = new DataMap();
    result->insert("name", new DataValue(name));

    result->insert("settings", createClusterSettings());

    result->insert("segments", createSegments(numberOfInputNodes,
                                              numberOfOutputNodes,
                                              settingsOverride));

    return result;
}


#endif // TEMPLATECREATOR_H
