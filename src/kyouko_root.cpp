/**
 * @file        kyouko_root.cpp
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

#include <kyouko_root.h>

#include <core/network_manager.h>
#include <core/objects/segment.h>
#include <core/objects/network_cluster.h>
#include <core/objects/output.h>
#include <core/validation.h>
#include <core/connection_handler/client_connection_handler.h>
#include <core/connection_handler/monitoring_connection_handler.h>
#include <core/processing/cpu/output_synapse_processing.h>
#include <core/processing/cpu/synapse_processing.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>
#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

// init static variables
KyoukoRoot* KyoukoRoot::m_root = nullptr;
NetworkCluster* KyoukoRoot::m_networkCluster = nullptr;
bool KyoukoRoot::m_freezeState = false;
ClientConnectionHandler* KyoukoRoot::m_clientHandler = nullptr;
MonitoringConnectionHandler* KyoukoRoot::m_monitoringHandler = nullptr;
InputOutputProcessing* KyoukoRoot::m_ioHandler = nullptr;

/**
 * @brief KyoukoRoot::KyoukoRoot
 */
KyoukoRoot::KyoukoRoot()
{
    validateStructSizes();

    m_root = this;
    m_freezeState = false;
    m_clientHandler = new ClientConnectionHandler();
    m_monitoringHandler = new MonitoringConnectionHandler();
}

/**
 * @brief KyoukoRoot::initializeSakuraFiles
 * @return
 */
bool
KyoukoRoot::initializeSakuraFiles()
{
    bool success = false;
    const std::string sakuraDir = GET_STRING_CONFIG("DEFAULT", "sakura-file-locaion", success);
    if(success == false) {
        return false;
    }

    std::string errorMessage = "";
    success = SakuraLangInterface::getInstance()->readFilesInDir(sakuraDir, errorMessage);
    if(success == false)
    {
        LOG_ERROR(errorMessage);
        return false;
    }

    return true;
}

/**
 * @brief KyoukoRoot::~KyoukoRoot
 */
KyoukoRoot::~KyoukoRoot() {}

/**
 * init all components
 */
bool
KyoukoRoot::start()
{
    // network-manager
    m_networkManager = new NetworkManager();
    //m_networkManager->startThread();

    return true;
}


bool
KyoukoRoot::learnStep()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    uint32_t timeout = 50;

    cluster->networkMetaData.doLearn = 1;

    //----------------------------------------------------------------------------------------------
    // learn phase 1
    timeout = 5;
    uint32_t updateVals = 0;
    uint32_t tempVal = 0;
    do
    {
        executeStep();

        InputNode* inputNodes = cluster->synapseSegment->inputNodes;
        for(uint32_t i = 0; i < 800; i++) {
            inputNodes[i].weight = m_inputBuffer[i];
        }

        executeStep();
        tempVal = output_precheck(cluster->outputSegment);
        std::cout<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++: "<<tempVal<<std::endl;

        if(tempVal < updateVals)
        {
            updateVals = tempVal;
            break;
        }
        updateVals = tempVal;

        timeout--;
    }
    while(updateVals != 0
          && timeout > 0);


    if(updateVals == 0)
    {
        KyoukoRoot::m_freezeState = true;
        cluster->networkMetaData.lerningValue = 100000.0f;

        executeStep();

        finishStep();

        return true;
    }

    //----------------------------------------------------------------------------------------------
    // learn phase 2
    timeout = 50;
    float totalDiff = 0.0f;
    do
    {
        totalDiff = output_learn_step(cluster->outputSegment, &cluster->networkMetaData);
        timeout--;
    }
    while(totalDiff >= 1.0f
          && timeout > 0);
    std::cout<<"###################################################: "<<totalDiff<<std::endl;

    // if desired state was reached, than freeze lerned state
    if(totalDiff < 1.0f)
    {
        KyoukoRoot::m_freezeState = true;
        cluster->networkMetaData.lerningValue = 100000.0f;
        executeStep();
    }

    //----------------------------------------------------------------------------------------------

    finishStep();

    return true;
}

void KyoukoRoot::executeStep()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;

    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    float timeValue = 0.0f;
    SynapseSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;
    OutputSegment* outputSegment = KyoukoRoot::m_networkCluster->outputSegment;

    // learn until output-section
    const uint32_t runCount = cluster->initMetaData.layer + 2;
    for(uint32_t i = 0; i < runCount; i++)
    {
        start = std::chrono::system_clock::now();
        node_processing(synapseSegment->nodes,
                        synapseSegment->nodeBuffers,
                        synapseSegment->inputNodes,
                        synapseSegment->synapseBuffers,
                        synapseSegment->segmentMeta,
                        synapseSegment->synapseMetaData,
                        outputSegment->inputs);
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        //std::cout<<"node-time: "<<(timeValue / 1000.0f)<<" us"<<std::endl;

        start = std::chrono::system_clock::now();
        synapse_processing(synapseSegment->segmentMeta,
                           synapseSegment->synapseBuffers,
                           synapseSegment->synapseSections,
                           synapseSegment->nodes,
                           synapseSegment->nodeBricks,
                           synapseSegment->nodeBuffers,
                           synapseSegment->synapseMetaData,
                           &KyoukoRoot::m_networkCluster->networkMetaData);
        end = std::chrono::system_clock::now();
        timeValue = std::chrono::duration_cast<chronoNanoSec>(end - start).count();
        //std::cout<<"synapse-time: "<<(timeValue / 1000.0f)<<" us"<<std::endl;

        //KyoukoRoot::m_root->m_networkManager->executeStep();
    }

    output_node_processing(cluster->outputSegment, &cluster->networkMetaData);
}

void KyoukoRoot::finishStep()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;

    cluster->networkMetaData.doLearn = 0;
    cluster->networkMetaData.lerningValue = 0.0f;

    KyoukoRoot::m_freezeState = false;

    InputNode* inputNodes = cluster->synapseSegment->inputNodes;
    for(uint32_t i = 0; i < 800; i++) {
        inputNodes[i].weight = 0.0f;
    }

    // reset network
    executeStep();
}

void KyoukoRoot::learnTestData()
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;

    const std::string trainDataPath = "/home/neptune/Schreibtisch/mnist/train-images.idx3-ubyte";
    const std::string trainLabelPath = "/home/neptune/Schreibtisch/mnist/train-labels.idx1-ubyte";
    const std::string testDataPath = "/home/neptune/Schreibtisch/mnist/t10k-images.idx3-ubyte";
    const std::string testLabelPath = "/home/neptune/Schreibtisch/mnist/t10k-labels.idx1-ubyte";

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

    for(uint32_t i = 0; i < 800; i++) {
        m_inputBuffer[i] = 0.0f;
    }

    std::cout<<"learn"<<std::endl;

    for(uint32_t poi = 0; poi < 1; poi++)
    {
        for(uint32_t pic = 0; pic < 60000; pic++)
        {
            const uint32_t label = labelBufferPtr[pic + 8];
            std::cout<<"picture: "<<pic<<std::endl;

            Output* outputs = cluster->outputSegment->outputs;
            for(uint32_t i = 0; i < 10; i++) {
                outputs[i].shouldValue = 0.0f;
            }

            outputs[label].shouldValue = 255.0f;
            std::cout<<"label: "<<label<<std::endl;

            for(uint32_t i = 0; i < pictureSize; i++)
            {
                const uint32_t pos = pic * pictureSize + i + 16;
                int32_t total = dataBufferPtr[pos] * 1;
                m_inputBuffer[i] = (static_cast<float>(total));
            }

            /*for(uint32_t i = 0; i < pictureSize; i = i + 2)
            {
                const uint32_t pos = pic * pictureSize + i + 16;
                int32_t total = dataBufferPtr[pos] * 5 + dataBufferPtr[pos + 1] * 5;
                m_inputBuffer[i/2] = (static_cast<float>(total));
            }

            for(uint32_t x = 4; x < 20; x++)
            {
                for(uint32_t y = 4; y < 20; y = y + 2)
                {
                    const uint32_t pixelPos = x * 28 + y;
                    const uint32_t pos = pic * pictureSize + pixelPos + 16;
                    int32_t total = (255 - dataBufferPtr[pos]) * 5 + (255 - dataBufferPtr[pos + 1]) * 5;
                    if(total < 0) {
                        total = 0;
                    }
                    m_inputBuffer[(pictureSize / 2) + (pixelPos/2)] = (static_cast<float>(total));
                }
            }*/

            KyoukoRoot::m_root->learnStep();
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

    InputNode* inputNodes = cluster->synapseSegment->inputNodes;
    for(uint32_t i = 0; i < 800; i = i + 2)  {
        inputNodes[i].weight = 0.0f;
    }


    for(uint32_t pic = 0; pic < total; pic++)
    {
        uint32_t label = testLabelBufferPtr[pic + 8];

        //std::cout<<pic<<" should: "<<(int)labelBufferPtr[pic + 8]<<"   is: ";
        std::cout<<pic<<" should: "<<label<<"   is: ";

        for(uint32_t i = 0; i < pictureSize; i++)
        {
            const uint32_t pos = pic * pictureSize + i + 16;
            int32_t total = testDataBufferPtr[pos] * 1;
            inputNodes[i].weight = (static_cast<float>(total));
        }

        /*for(uint32_t i = 0; i < pictureSize; i = i + 2)
        {
            const uint32_t pos = pic * pictureSize + i + 16;
            int32_t total = testDataBufferPtr[pos] * 5 +  testDataBufferPtr[pos + 1] * 5;
            inputNodes[(i/2) + inputBrick->nodePos] = (static_cast<float>(total));
        }

        for(uint32_t x = 4; x < 20; x++)
        {
            for(uint32_t y = 4; y < 20; y = y + 2)
            {
                const uint32_t pixelPos = x * 28 + y;
                const uint32_t pos = pic * pictureSize + pixelPos + 16;
                int32_t total = (255 - testDataBufferPtr[pos]) * 5 + (255 - testDataBufferPtr[pos + 1]) * 5;
                if(total < 0) {
                    total = 0;
                }
                inputNodes[(pictureSize / 2) + (pixelPos/2) + inputBrick->nodePos] = (static_cast<float>(total));
            }
        }*/

        executeStep();

        // print result
        float biggest = -100000.0f;
        uint32_t pos = 0;
        Output* outputs = cluster->outputSegment->outputs;
        std::string outString = "[";
        for(uint32_t i = 0; i < cluster->outputSegment->segmentMeta->numberOfOutputs; i++)
        {
            if(i > 0) {
                outString += " | ";
            }
            const float read = outputs[i].outputValue;
            outString += std::to_string((int)read) + "\t";
            if(read > biggest)
            {
                biggest = read;
                pos = i;
            }
        }
        outString += "]";
        std::cout<<pos<<"   complete: "<<outString<<std::endl;

        if(testLabelBufferPtr[pic + 8] == pos
                && biggest != 0.0f)
        {
            match++;
        }
    }

    std::cout<<"======================================================================="<<std::endl;
    std::cout<<"correct: "<<match<<"/"<<total<<std::endl;
    std::cout<<"======================================================================="<<std::endl;
}

