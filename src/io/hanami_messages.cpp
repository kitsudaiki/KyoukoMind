/**
 * @file        hanami_messages.cpp
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

#include "hanami_messages.h"

#include <core/cluster/cluster.h>
#include <../libKitsunemimiHanamiMessages/hanami_messages/kyouko_messages.h>
#include <libKitsunemimiHanamiMessaging/hanami_messaging_client.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>

/**
 * @brief send output of an output-segment as hanami-message
 *
 * @param segment segment, which output-data should send
 */
void
sendHanamiOutputMessage(const OutputSegment &segment)
{
    if(segment.parentCluster->msgClient == nullptr) {
        return;
    }

    OutputNode* node = nullptr;

    // filter values for only necessary values
    float outputData[segment.segmentHeader->outputs.count];
    for(uint64_t outputNodeId = 0;
        outputNodeId < segment.segmentHeader->outputs.count;
        outputNodeId++)
    {
        node = &segment.outputs[outputNodeId];
        outputData[outputNodeId] = node->outputWeight;
    }

    // build message
    Kitsunemimi::Hanami::ClusterIO_Message msg;
    msg.segmentName = segment.getName();
    msg.isLast = false;
    msg.processType = Kitsunemimi::Hanami::ClusterIO_Message::ProcessType::REQUEST_TYPE;
    msg.dataType = Kitsunemimi::Hanami::ClusterIO_Message::DataType::OUTPUT_TYPE;
    msg.numberOfValues = segment.segmentHeader->outputs.count;
    msg.values = outputData;

    // serialize message
    uint8_t buffer[96*1024];
    const uint64_t size = msg.createBlob(buffer, 96*1024);
    if(size == 0)
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

void
sendHanamiNormalEndMessage(Cluster* cluster)
{
    if(cluster->msgClient == nullptr) {
        return;
    }

    // build message
    Kitsunemimi::Hanami::ClusterIO_Message msg;
    msg.isLast = true;
    msg.processType = Kitsunemimi::Hanami::ClusterIO_Message::ProcessType::REQUEST_TYPE;
    msg.dataType = Kitsunemimi::Hanami::ClusterIO_Message::DataType::OUTPUT_TYPE;

    // serialize message
    uint8_t buffer[96*1024];
    const uint64_t size = msg.createBlob(buffer, 96*1024);
    if(size == 0)
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("Failed to serialize request-message");
        return;
    }

    // send message
    Kitsunemimi::ErrorContainer error;
    cluster->msgClient->sendStreamMessage(buffer, size, false, error);
}

void
sendHanamiLearnEndMessage(Cluster* cluster)
{
    if(cluster->msgClient == nullptr) {
        return;
    }

    // build message
    Kitsunemimi::Hanami::ClusterIO_Message msg;
    msg.isLast = true;
    msg.processType = Kitsunemimi::Hanami::ClusterIO_Message::ProcessType::LEARN_TYPE;
    msg.dataType = Kitsunemimi::Hanami::ClusterIO_Message::DataType::OUTPUT_TYPE;

    // serialize message
    uint8_t buffer[96*1024];
    const uint64_t size = msg.createBlob(buffer, 96*1024);
    if(size == 0)
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
 * @brief process incoming data as hanami-message
 *
 * @param cluster cluster which receive the data
 * @param data incoming data
 * @param dataSize incoming number of bytes
 *
 * @return false, if message is broken, else true
 */
bool
recvHanamiInputMessage(Cluster* cluster,
                       const void* data,
                       const uint64_t dataSize)
{
    // parse incoming data
    Kitsunemimi::Hanami::ClusterIO_Message msg;
    if(msg.read(const_cast<void*>(data), dataSize) == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("Got invalid Hanami-ClusterIO-Message");
        LOG_ERROR(error);
        return false;
    }

    // fill given data into the target-segment
    if(msg.dataType == Kitsunemimi::Hanami::ClusterIO_Message::DataType::INPUT_TYPE)
    {
        std::map<std::string, InputSegment*>::iterator it;
        it = cluster->inputSegments.find(msg.segmentName);
        if(it != cluster->inputSegments.end())
        {
            InputNode* inputNodes = it->second->inputs;
            for(uint64_t i = 0; i < msg.numberOfValues; i++) {
                inputNodes[i].weight = msg.values[i];
            }
        }
    }
    if(msg.dataType == Kitsunemimi::Hanami::ClusterIO_Message::DataType::SHOULD_TYPE)
    {
        std::map<std::string, OutputSegment*>::iterator it;
        it = cluster->outputSegments.find(msg.segmentName);
        if(it != cluster->outputSegments.end())
        {
            OutputNode* outputNodes = it->second->outputs;
            for(uint64_t i = 0; i < msg.numberOfValues; i++) {
                outputNodes[i].shouldValue = msg.values[i];
            }
        }
    }

    // mark that a protobuf-response is NOT required
    cluster->useProtobuf = false;

    if(msg.isLast)
    {
        // start request
        if(msg.processType == Kitsunemimi::Hanami::ClusterIO_Message::ProcessType::REQUEST_TYPE)
        {
            cluster->mode = Cluster::NORMAL_MODE;
            cluster->startForwardCycle();
        }

        // start learn
        if(msg.processType == Kitsunemimi::Hanami::ClusterIO_Message::ProcessType::LEARN_TYPE)
        {
            cluster->mode = Cluster::LEARN_FORWARD_MODE;
            cluster->startForwardCycle();
        }
    }

    return true;
}
