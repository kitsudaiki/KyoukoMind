/**
 * @file        protobuf_messages.cpp
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

#include "protobuf_messages.h"

#include <core/cluster/cluster.h>
#include <../libKitsunemimiHanamiMessages/protobuffers/kyouko_messages.proto3.pb.h>
#include <libKitsunemimiHanamiNetwork/hanami_messaging_client.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>

/**
 * @brief send output of an output-segment as protobuf-message
 *
 * @param segment segment, which output-data should send
 */
void
sendProtobufOutputMessage(const OutputSegment &segment)
{
    if(segment.parentCluster->msgClient == nullptr) {
        return;
    }

    // build message
    ClusterIO_Message msg;
    msg.set_segmentname(segment.getName());
    msg.set_islast(false);
    msg.set_processtype(ClusterProcessType::REQUEST_TYPE);
    msg.set_datatype(ClusterDataType::OUTPUT_TYPE);
    msg.set_numberofvalues(segment.segmentHeader->outputs.count);

    for(uint64_t outputNodeId = 0;
        outputNodeId < segment.segmentHeader->outputs.count;
        outputNodeId++)
    {
        msg.add_values(segment.outputs[outputNodeId].outputWeight);
    }

    // serialize message
    uint8_t buffer[96*1024];
    const uint64_t size = msg.ByteSizeLong();
    if(msg.SerializeToArray(buffer, size) == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("Failed to serialize request-message");
        return;
    }

    // send message
    Kitsunemimi::Hanami::HanamiMessagingClient* client = segment.parentCluster->msgClient;
    Kitsunemimi::ErrorContainer error;
    client->sendStreamMessage(buffer, size, false, error);
}

/**
 * @brief sendProtobufNormalEndMessage
 * @param cluster
 */
void
sendProtobufNormalEndMessage(Cluster* cluster)
{
    if(cluster->msgClient == nullptr) {
        return;
    }

    // build message
    ClusterIO_Message msg;
    msg.set_islast(true);
    msg.set_processtype(ClusterProcessType::REQUEST_TYPE);
    msg.set_datatype(ClusterDataType::OUTPUT_TYPE);

    // serialize message
    uint8_t buffer[96*1024];
    const uint64_t size = msg.ByteSizeLong();
    if(msg.SerializeToArray(buffer, size) == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("Failed to serialize request-message");
        return;
    }

    // send message
    Kitsunemimi::ErrorContainer error;
    cluster->msgClient->sendStreamMessage(buffer, size, false, error);
}

/**
 * @brief sendProtobufLearnEndMessage
 * @param cluster
 */
void
sendProtobufLearnEndMessage(Cluster* cluster)
{
    if(cluster->msgClient == nullptr) {
        return;
    }

    // build message
    ClusterIO_Message msg;
    msg.set_islast(true);
    msg.set_processtype(ClusterProcessType::LEARN_TYPE);
    msg.set_datatype(ClusterDataType::OUTPUT_TYPE);

    // serialize message
    uint8_t buffer[96*1024];
    const uint64_t size = msg.ByteSizeLong();
    if(msg.SerializeToArray(buffer, size) == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("Failed to serialize request-message");
        return;
    }

    // send message
    Kitsunemimi::ErrorContainer error;
    cluster->msgClient->sendStreamMessage(buffer, size, false, error);
}

/**
 * @brief process incoming data as protobuf-message
 *
 * @param cluster cluster which receive the data
 * @param data incoming data
 * @param dataSize incoming number of bytes
 *
 * @return false, if message is broken, else true
 */
bool
recvProtobufInputMessage(Cluster* cluster,
                         const void* data,
                         const uint64_t dataSize)
{
    // parse incoming data
    ClusterIO_Message msg;
    if(msg.ParseFromArray(data, dataSize) == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("Got invalid Protobuf-ClusterIO-Message");
        LOG_ERROR(error);
        return false;
    }

    // fill given data into the target-segment
    if(msg.datatype() == ClusterDataType::INPUT_TYPE)
    {
        std::map<std::string, InputSegment*>::iterator it;
        it = cluster->inputSegments.find(msg.segmentname());
        if(it != cluster->inputSegments.end())
        {
            InputNode* inputNodes = it->second->inputs;
            for(uint64_t i = 0; i < msg.numberofvalues(); i++) {
                inputNodes[i].weight = msg.values(i);
            }
        }
    }
    if(msg.datatype() == ClusterDataType::SHOULD_TYPE)
    {
        std::map<std::string, OutputSegment*>::iterator it;
        it = cluster->outputSegments.find(msg.segmentname());
        if(it != cluster->outputSegments.end())
        {
            OutputNode* outputNodes = it->second->outputs;
            for(uint64_t i = 0; i < msg.numberofvalues(); i++) {
                outputNodes[i].shouldValue = msg.values(i);
            }
        }
    }

    // mark that a protobuf-response is required
    cluster->useProtobuf = true;

    if(msg.islast())
    {
        // start request
        if(msg.processtype() == ClusterProcessType::REQUEST_TYPE)
        {
            cluster->mode = Cluster::NORMAL_MODE;
            cluster->startForwardCycle();
        }

        // start learn
        if(msg.processtype() == ClusterProcessType::LEARN_TYPE)
        {
            cluster->mode = Cluster::LEARN_FORWARD_MODE;
            cluster->startForwardCycle();
        }
    }

    return true;
}
