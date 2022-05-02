/**
 * @file        dynamic_segments.h
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

#ifndef DYNAMIC_SEGMENTS_H
#define DYNAMIC_SEGMENTS_H

#include <common.h>

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
createPosition(const long x, const long y, const long z)
{
    DataArray* position = new DataArray();

    position->append(new DataValue(x));
    position->append(new DataValue(y));
    position->append(new DataValue(z));

    return position;
}

/**
 * @brief create settings block for the segment
 *
 * @param result pointer to the resulting object
 */
DataMap*
createSegmentSettings(const JsonItem &settingsOverride)
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
    settings->insert("max_synapse_sections", new DataValue(100000));

    const std::vector<std::string> keys = settingsOverride.getKeys();
    for(const std::string &key : keys) {
        settings->insert(key, settingsOverride.get(key).getItemContent()->copy(), true);
    }

    return settings;
}

/**
 * @brief create bricks for an internal segment
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 */
DataArray*
createSegmentBricksGraph(const long numberOfInputNodes,
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
    DataMap* internalBrick1 = new DataMap();
    internalBrick1->insert("type", new DataValue("normal"));
    internalBrick1->insert("number_of_nodes", new DataValue(5000));
    internalBrick1->insert("position", createPosition(2, 1, 1));
    bricks->append(internalBrick1);

    // centre part
    DataMap* internalBrick2 = new DataMap();
    internalBrick2->insert("type", new DataValue("normal"));
    internalBrick2->insert("number_of_nodes", new DataValue(1000));
    internalBrick2->insert("position", createPosition(3, 1, 1));
    bricks->append(internalBrick2);

    // output-part
    DataMap* outputBrick = new DataMap();
    outputBrick->insert("type", new DataValue("output"));
    outputBrick->insert("number_of_nodes", new DataValue(numberOfOutputNodes));
    outputBrick->insert("position", createPosition(4, 1, 1));
    bricks->append(outputBrick);

    return bricks;
}

/**
 * @brief create bricks for an internal segment
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 */
DataArray*
createSegmentBricksImage(const long numberOfInputNodes,
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

    return bricks;
}

/**
 * @brief create internal dynamic segments
 *
 * @param result pointer to the resulting object
 * @param numberOfInputNodes number of input-nodes of the cluster
 * @param numberOfOutputNodes number of output-nodes of the cluster
 */
DataMap*
createDynamicSegments(const long numberOfInputNodes,
                      const long numberOfOutputNodes,
                      const std::string &type,
                      const JsonItem &settingsOverride)
{
    DataMap* newSegment = new DataMap();

    //newSegment->insert("type", new DataValue("static_segment"));
    newSegment->insert("type", new DataValue("dynamic_segment"));
    newSegment->insert("position", createPosition(1, 2, 1));

    newSegment->insert("settings", createSegmentSettings(settingsOverride));
    if(type == "image")
    {
        newSegment->insert("bricks", createSegmentBricksImage(numberOfInputNodes,
                                                              numberOfOutputNodes));
    }
    else if(type == "graph")
    {
        newSegment->insert("bricks", createSegmentBricksGraph(numberOfInputNodes,
                                                              numberOfOutputNodes));
    }

    return newSegment;
}

#endif // DYNAMIC_SEGMENTS_H
