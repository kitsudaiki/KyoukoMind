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

#include <core/validation.h>
#include <core/connection_handler/client_connection_handler.h>
#include <core/connection_handler/monitoring_connection_handler.h>
#include <core/processing/static_processing/single_thread_processing_static.h>
#include <core/objects/network_cluster.h>
#include <core/objects/node.h>
#include <core/storage_io.h>

#include <initializing/network_initializer.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>
#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

// init static variables
KyoukoRoot* KyoukoRoot::m_root = nullptr;
NetworkCluster* KyoukoRoot::m_networkCluster = nullptr;
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
    NetworkInitializer initializer;
    return initializer.initNetwork();
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

    m_staticProcessing = new SingleThreadProcessingStatic();


    // get pictures
    const uint32_t pictureSize = numberOfRows * numberOfColumns;
    InputNode* inputNodes = cluster->synapseSegment->inputNodes;
    for(uint32_t i = 0; i < 800; i++)  {
        inputNodes[i].weight = 0.0f;
    }

    std::cout<<"learn"<<std::endl;

    for(uint32_t poi = 0; poi < 1; poi++)
    {
        for(uint32_t pic = 0; pic < 60000; pic++)
        {
            const uint32_t label = labelBufferPtr[pic + 8];
            std::cout<<"picture: "<<pic<<std::endl;

            OutputNode* outputs = cluster->synapseSegment->outputNodes;
            for(uint32_t i = 0; i < 10; i++) {
                outputs[i].shouldValue = 0.0f;
            }

            outputs[label].shouldValue = 1.0f;
            std::cout<<"label: "<<label<<std::endl;

            for(uint32_t i = 0; i < pictureSize; i++)
            {
                const uint32_t pos = pic * pictureSize + i + 16;
                int32_t total = dataBufferPtr[pos];
                m_staticProcessing->buffer[i] = (static_cast<float>(total) / 255.0f);
            }

            m_staticProcessing->learn();
        }
    }

    //return;

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

    for(uint32_t i = 0; i < 800; i++)  {
        inputNodes[i].weight = 0.0f;
    }

    CoreSegment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    for(uint32_t pic = 0; pic < total; pic++)
    {
        uint32_t label = testLabelBufferPtr[pic + 8];

        std::cout<<pic<<" should: "<<label<<"   is: ";

        for(uint32_t i = 0; i < pictureSize; i++)
        {
            const uint32_t pos = pic * pictureSize + i + 16;
            int32_t total = testDataBufferPtr[pos];
            inputNodes[i].weight = (static_cast<float>(total));
        }

        m_staticProcessing->execute();

        // print result
        float biggest = -100000.0f;
        uint32_t pos = 0;
        OutputNode* outputs = cluster->synapseSegment->outputNodes;
        Node* nodes = cluster->synapseSegment->nodes;
        std::cout<<"[";


        for(uint64_t i = 0; i < synapseSegment->segmentMeta->numberOfOutputs; i++)
        {
            OutputNode* out = &synapseSegment->outputNodes[i];

            if(i > 0) {
                std::cout<<" | ";
            }

            float read = out->outputWeight;

            std::cout.precision(3);
            if(read < 0.001f) {
                read = 0.0f;
            }
            std::cout<<read<<"\t";

            if(read > biggest)
            {
                biggest = read;
                pos = i;
            }
        }

        std::cout<<"]  result: ";
        std::cout<<pos;

        if(testLabelBufferPtr[pic + 8] == pos) {
            match++;
        } else {
            std::cout<<"     FAIL!!!!!!!";
        }
        std::cout<<std::endl;
    }

    std::cout<<"======================================================================="<<std::endl;
    std::cout<<"correct: "<<match<<"/"<<total<<std::endl;
    std::cout<<"======================================================================="<<std::endl;

    StorageIO io;
    io.writeToDisc("/tmp/");
}
