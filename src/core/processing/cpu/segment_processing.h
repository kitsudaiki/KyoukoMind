/**
 * @file        synapse_processing.h
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

#ifndef SEGMENT_PROCESSING_H
#define SEGMENT_PROCESSING_H

#include <common.h>
#include <core/objects/segment.h>

void processSegmentInput(Segment* segment);
void processSegmentOutput(Segment* segment);
float calculateSegmentError(Segment* segment);

void reduceSegmentSynapses(Segment* segment);
void rewightSegment(Segment* segment);
void hardenSegment(Segment* segment);
void prcessSegmentNodes(Segment* segment);

#endif // SEGMENT_PROCESSING_H
