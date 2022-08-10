/**
 * @file        callbacks.cpp
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

#include <callbacks.h>

#include <core/cluster/cluster.h>
#include <core/segments/input_segment/input_segment.h>
#include <core/segments/output_segment/output_segment.h>

#include <libKitsunemimiSakuraNetwork/session.h>
#include <../libKitsunemimiHanamiProtobuffers/kyouko_messages.proto3.pb.h>

/**
 * @brief process stream-message
 *
 * @param target target-cluster of the session-endpoint
 * @param data incoming data
 * @param dataSize number of incoming data
 */
void
streamDataCallback(void* target,
                   Kitsunemimi::Sakura::Session*,
                   const void* data,
                   const uint64_t dataSize)
{
    Cluster* cluster = static_cast<Cluster*>(target);

    ClusterIO_Message msg;
    if(msg.ParseFromArray(data, dataSize) == false)
    {
        Kitsunemimi::ErrorContainer error;
        error.addMeesage("Got invalid ClusterIO-Message");
        LOG_ERROR(error);
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

    /**std::cout<<"#################################################"<<std::endl;
    std::cout<<"number of values: "<<msg.numberOfValues<<std::endl;
    std::cout<<"val0: "<<msg.values[0]<<std::endl;
    std::cout<<"val1: "<<msg.values[1]<<std::endl;
    std::cout<<"#################################################"<<std::endl;*/
}
