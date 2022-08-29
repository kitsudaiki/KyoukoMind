/**
 * @file        protobuf_messages.h
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

#ifndef KYOUKOMIND_PROTOBUF_MESSAGES_H
#define KYOUKOMIND_PROTOBUF_MESSAGES_H

#include <core/segments/output_segment/output_segment.h>

void sendProtobufOutputMessage(const OutputSegment &segment);
void sendProtobufNormalEndMessage(Cluster* cluster);
void sendProtobufLearnEndMessage(Cluster* cluster);

bool recvProtobufInputMessage(Cluster* cluster,
                              const void* data,
                              const uint64_t dataSize);


#endif // KYOUKOMIND_PROTOBUF_MESSAGES_H
