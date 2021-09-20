/**
 * @file        dev_test.h
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

#include "dev_test.h"

#include <kyouko_root.h>
#include <core/objects/network_cluster.h>
#include <core/objects/segments/input_segment.h>
#include <core/objects/segments/output_segment.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/text_file.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCommon/progress_bar.h>

#include <core/processing/cpu/cpu_processing_unit.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <core/cluster_handler.h>

#include <core/processing/task_queue.h>

/**
 * @brief only a test-function for fast tests
 *
 * @param mnistRootPath absolute path to the directory with the MNIST test-files
 */
void
learnTestData(const std::string &mnistRootPath,
              const std::string &uuid)
{
    NetworkCluster* cluster = KyoukoRoot::m_root->m_clusterHandler->getCluster(uuid);
    CpuProcessingUnit cpuProcessingUnit;
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    bool success = false;

    // /home/neptune/Schreibtisch/mnist

    const std::string trainDataPath = mnistRootPath + "/train-images.idx3-ubyte";
    const std::string trainLabelPath = mnistRootPath + "/train-labels.idx1-ubyte";
    const std::string testDataPath = mnistRootPath + "/t10k-images.idx3-ubyte";
    const std::string testLabelPath = mnistRootPath + "/t10k-labels.idx1-ubyte";

    //==============================================================================================
    // learn
    //==============================================================================================

    // read train-data
    Kitsunemimi::Persistence::BinaryFile trainData(trainDataPath);
    Kitsunemimi::DataBuffer trainDataBuffer;
    trainData.readCompleteFile(trainDataBuffer);

    // read train-labels
    Kitsunemimi::Persistence::BinaryFile trainLabel(trainLabelPath);
    Kitsunemimi::DataBuffer trainLabelBuffer;
    trainLabel.readCompleteFile(trainLabelBuffer);

    std::cout<<trainDataBuffer.bufferPosition<<std::endl;
    std::cout<<trainLabelBuffer.bufferPosition<<std::endl;

    uint8_t* dataBufferPtr = static_cast<uint8_t*>(trainDataBuffer.data);
    uint8_t* labelBufferPtr = static_cast<uint8_t*>(trainLabelBuffer.data);

    // get number of images
    uint32_t numberOfImages = 0;
    numberOfImages |= dataBufferPtr[7];
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[6]) << 8;
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[5]) << 16;
    numberOfImages |= static_cast<uint32_t>(dataBufferPtr[4]) << 24;
    std::cout<<"number of images: "<<numberOfImages<<std::endl;

    // get number of rows
    uint32_t numberOfRows = 0;
    numberOfRows |= dataBufferPtr[11];
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[10]) << 8;
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[9]) << 16;
    numberOfRows |= static_cast<uint32_t>(dataBufferPtr[8]) << 24;
    std::cout<<"number of rows: "<<numberOfRows<<std::endl;

    // get number of columns
    uint32_t numberOfColumns = 0;
    numberOfColumns |= dataBufferPtr[15];
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[14]) << 8;
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[13]) << 16;
    numberOfColumns |= static_cast<uint32_t>(dataBufferPtr[12]) << 24;
    std::cout<<"number of columns: "<<numberOfColumns<<std::endl;

    // get pictures
    const uint32_t pictureSize = numberOfRows * numberOfColumns;

    std::cout<<"learn"<<std::endl;

    const uint32_t numberOfIteractions = GET_INT_CONFIG("DevMode", "learn_iterations", success);
    const uint32_t numberOfLearningPictures = GET_INT_CONFIG("DevMode", "learn_images", success);

    for(uint32_t iter = 0; iter < 5; iter++)
    {
        for(uint32_t poi = 0; poi < 1; poi++)
        {
            uint64_t dataPos = 0;
            uint64_t dataSize = numberOfLearningPictures * (pictureSize + 10);
            float* taskData = new float[dataSize];

            for(uint32_t pic = 0; pic < numberOfLearningPictures; pic++)
            {
                // input
                for(uint32_t i = 0; i < pictureSize; i++)
                {
                    const uint32_t pos = pic * pictureSize + i + 16;
                    int32_t total = dataBufferPtr[pos];
                    taskData[dataPos] = (static_cast<float>(total) / 255.0f);
                    dataPos++;
                }

                // output
                for(uint32_t i = 0; i < 10; i++)
                {
                    taskData[dataPos] = 0.0f;
                    dataPos++;
                }
                const uint32_t label = labelBufferPtr[pic + 8];
                taskData[(dataPos - 10) + label] = 1.0f;
            }

            // create task
            const std::string taskUuid = cluster->taskQueue->addLearnTask(taskData,
                                                                          pictureSize,
                                                                          10,
                                                                          numberOfLearningPictures);
            cluster->updateClusterState();

            // wait until task is finished
            start = std::chrono::system_clock::now();
            Kitsunemimi::ProgressBar* progressBar = new Kitsunemimi::ProgressBar();
            while(cluster->taskQueue->isFinish(taskUuid) == false) {
                progressBar->updateProgress(cluster->taskQueue->actualTask->progress.percentageFinished);
                usleep(100000);
            }
            progressBar->updateProgress(1.0f);
            end = std::chrono::system_clock::now();
            const float time = std::chrono::duration_cast<chronoSec>(end - start).count();
            delete progressBar;
            std::cout<<"run learn: "<<time<<"s"<<std::endl;
        }
    }

    //==============================================================================================
    // test
    //==============================================================================================

    // read train-data
    Kitsunemimi::Persistence::BinaryFile testData(testDataPath);
    Kitsunemimi::DataBuffer testDataBuffer;
    testData.readCompleteFile(testDataBuffer);

    // read train-labels
    Kitsunemimi::Persistence::BinaryFile testLabel(testLabelPath);
    Kitsunemimi::DataBuffer testLabelBuffer;
    testLabel.readCompleteFile(testLabelBuffer);

    uint8_t* testDataBufferPtr = static_cast<uint8_t*>(testDataBuffer.data);
    uint8_t* testLabelBufferPtr = static_cast<uint8_t*>(testLabelBuffer.data);

    std::cout<<"test"<<std::endl;
    uint32_t match = 0;
    uint32_t total = 10000;

    uint64_t dataPos = 0;
    uint64_t dataSize = numberOfLearningPictures * (pictureSize + 10);
    float* taskData = new float[dataSize];

    for(uint32_t pic = 0; pic < total; pic++)
    {
        // input
        for(uint32_t i = 0; i < pictureSize; i++)
        {
            const uint32_t pos = pic * pictureSize + i + 16;
            int32_t total = testDataBufferPtr[pos];
            taskData[dataPos] = (static_cast<float>(total) / 255.0f);
            dataPos++;
        }
    }

    // create task
    const std::string taskUuid = cluster->taskQueue->addRequestTask(taskData,
                                                                    pictureSize,
                                                                    total);
    cluster->updateClusterState();
    // wait until task is finished
    start = std::chrono::system_clock::now();
    Kitsunemimi::ProgressBar* progressBar = new Kitsunemimi::ProgressBar();
    while(cluster->taskQueue->isFinish(taskUuid) == false) {
        progressBar->updateProgress(cluster->taskQueue->actualTask->progress.percentageFinished);
        usleep(100000);
    }
    progressBar->updateProgress(1.0f);
    end = std::chrono::system_clock::now();
    const float time = std::chrono::duration_cast<chronoSec>(end - start).count();
    delete progressBar;
    std::cout<<"run request: "<<time<<"s"<<std::endl;


    const uint32_t* resultData = cluster->taskQueue->getResultData(taskUuid);

    for(uint32_t pic = 0; pic < total; pic++)
    {
        uint32_t label = testLabelBufferPtr[pic + 8];

        if(resultData[pic] == label) {
            match++;
        }
    }

    std::cout<<"======================================================================="<<std::endl;
    std::cout<<"correct: "<<match<<"/"<<total<<std::endl;
    std::cout<<"======================================================================="<<std::endl;

    DynamicSegment* segment = static_cast<DynamicSegment*>(cluster->allSegments.at(1));
    uint64_t synapseCounter = 0;
    SynapseSection* sections = segment->synapseSections;
    for(uint64_t i = 0; i < segment->segmentHeader->synapseSections.count; i++)
    {
        if(sections[i].active == Kitsunemimi::ItemBuffer::ACTIVE_SECTION)
        {
            for(uint32_t j = 0; j < SYNAPSES_PER_SYNAPSESECTION; j++)
            {
                if(sections[i].synapses[j].targetNodeId != UNINIT_STATE_16) {
                    synapseCounter++;
                }
            }
        }
    }
    std::cout<<std::endl;
    std::cout<<"======================================================================="<<std::endl;
    std::cout<<"synapseCounter: "<<synapseCounter<<std::endl;
    std::cout<<"======================================================================="<<std::endl;
}

