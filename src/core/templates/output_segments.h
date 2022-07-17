/**
 * @file        output_segments.h
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

#ifndef OUTPUT_SEGMENTS_H
#define OUTPUT_SEGMENTS_H

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
createOutputPosition(const long x, const long y, const long z)
{
    DataArray* position = new DataArray();

    position->append(new DataValue(x));
    position->append(new DataValue(y));
    position->append(new DataValue(z));

    return position;
}

/**
 * @brief create new output-segment
 *
 * @param result pointer to the resulting object
 * @param numberOfOutputNodes number of output-nodes of the cluster
 *
 * @return number of new output-segments
 */
DataMap*
createOutputSegments(const long numberOfOutputNodes)
{
    DataMap* newSegment = new DataMap();

    newSegment->insert("name", new DataValue("output"));
    newSegment->insert("type", new DataValue("output_segment"));
    newSegment->insert("position", createOutputPosition(1, 3, 1));
    newSegment->insert("number_of_outputs", new DataValue(numberOfOutputNodes));

    return newSegment;
}

#endif // OUTPUT_SEGMENTS_H
