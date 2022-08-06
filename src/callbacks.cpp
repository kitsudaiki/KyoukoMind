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
#include <libKitsumiAiSdk/messages/hanami_messages.h>

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
    const uint8_t* u8Data = static_cast<const uint8_t*>(data);
    Cluster* cluster = static_cast<Cluster*>(target);

    // insert data to the cluster
    if(u8Data[6] == 1)
    {
        void* normalData = const_cast<void*>(data);
        Kitsunemimi::Hanami::ClusterIO_Message msg;
        msg.read(normalData, dataSize);

        // fill given data into the target-segment
        if(msg.segmentType == Kitsunemimi::Hanami::ClusterIO_Message::INPUT_SEGMENT)
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
        else if(msg.segmentType == Kitsunemimi::Hanami::ClusterIO_Message::OUTPUT_SEGMENT)
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
    }

    // start request
    if(u8Data[6] == 2)
    {
        cluster->mode = Cluster::NORMAL_MODE;
        cluster->startForwardCycle();
    }

    // start learn
    if(u8Data[6] == 3)
    {
        cluster->mode = Cluster::LEARN_FORWARD_MODE;
        cluster->startForwardCycle();
    }

    /**std::cout<<"#################################################"<<std::endl;
    std::cout<<"number of values: "<<msg.numberOfValues<<std::endl;
    std::cout<<"val0: "<<msg.values[0]<<std::endl;
    std::cout<<"val1: "<<msg.values[1]<<std::endl;
    std::cout<<"#################################################"<<std::endl;*/
}
