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
#include <core/objects/global_values.h>
#include <core/objects/output.h>
#include <core/validation.h>
#include <core/processing/input_output_processing.h>
#include <core/connection_handler/client_connection_handler.h>
#include <core/connection_handler/monitoring_connection_handler.h>
#include <core/processing/cpu/output_synapse_processing.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiConfig/config_handler.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>
#include <libKitsunemimiSakuraMessaging/messaging_client.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

// init static variables
KyoukoRoot* KyoukoRoot::m_root = nullptr;
Segment* KyoukoRoot::m_segment = nullptr;
bool KyoukoRoot::m_freezeState = false;
ClientConnectionHandler* KyoukoRoot::m_clientHandler = nullptr;
MonitoringConnectionHandler* KyoukoRoot::m_monitoringHandler = nullptr;
InputOutputProcessing* KyoukoRoot::m_ioHandler = nullptr;

MonitoringBrickMessage KyoukoRoot::monitoringBrickMessage;
MonitoringProcessingTimes KyoukoRoot::monitoringMetaMessage;

/**
 * @brief KyoukoRoot::KyoukoRoot
 */
KyoukoRoot::KyoukoRoot()
{
    validateStructSizes();

    m_root = this;
    m_freezeState = false;
    m_segment = new Segment();
    m_ioHandler = new InputOutputProcessing();
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
    Segment* seg = KyoukoRoot::m_segment;
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(seg->globalValues);

    globalValue->doLearn = 1;

    // learn until output-section
    executeStep();

    // learn current state
    uint32_t timeout = 50;
    float totalDiff = 0.0f;
    do
    {
        totalDiff = output_learn_step();
        timeout--;
    }
    while(totalDiff >= 0.5f
          && timeout > 0);
    std::cout<<"###################################################: "<<totalDiff<<std::endl;

    bool result = false;

    // if desired state was reached, than freeze lerned state
    if(totalDiff < 0.5f)
    {
        result = true;
        KyoukoRoot::m_freezeState = true;
        globalValue->lerningValue = 100000.0f;
        executeStep();
        globalValue->doLearn = 0;
        globalValue->lerningValue = 0.0f;
        KyoukoRoot::m_freezeState = false;
    }

    // reset network
    KyoukoRoot::m_ioHandler->resetInput();
    KyoukoRoot::m_ioHandler->processInputMapping();
    KyoukoRoot::m_ioHandler->resetShouldValues();
    executeStep();

    return result;
}

void KyoukoRoot::executeStep()
{
    GlobalValues* globalValue = Kitsunemimi::getBuffer<GlobalValues>(KyoukoRoot::m_segment->globalValues);

    // learn until output-section
    const uint32_t runCount = globalValue->layer + 2;
    for(uint32_t i = 0; i < runCount; i++) {
        KyoukoRoot::m_root->m_networkManager->executeStep();
    }

    output_node_processing();
}

void KyoukoRoot::learnTestData()
{
    const std::string trainDataPath = "/home/neptune/Schreibtisch/mnist/train-images.idx3-ubyte";
    const std::string trainLabelPath = "/home/neptune/Schreibtisch/mnist/train-labels.idx1-ubyte";
    const std::string testDataPath = "/home/neptune/Schreibtisch/mnist/t10k-images.idx3-ubyte";
    const std::string testLabelPath = "/home/neptune/Schreibtisch/mnist/t10k-labels.idx1-ubyte";


    // register
    KyoukoRoot::m_ioHandler->registerInput(static_cast<uint32_t>(400));
    KyoukoRoot::m_ioHandler->registerOutput(static_cast<uint32_t>(10));


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
    for(uint32_t pic = 0; pic < 100; pic++)
    {
        std::cout<<"picture: "<<pic<<std::endl;

        Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
        for(uint32_t i = 0; i < 10; i++) {
            outputs[i].shouldValue = 0.0f;
        }

        const uint32_t label = labelBufferPtr[pic + 8];
        outputs[label].shouldValue = 255.0f;
        std::cout<<"label: "<<label<<std::endl;

        Brick* inputBrick = KyoukoRoot::m_segment->inputBricks[0];
        float* inputNodes = Kitsunemimi::getBuffer<float>(KyoukoRoot::m_segment->nodeInputBuffer);
        for(uint32_t i = 0; i < pictureSize; i = i + 2)
        {
            const uint32_t pos = pic * pictureSize + i + 16;
            uint32_t total = dataBufferPtr[pos] * 5 + dataBufferPtr[pos + 1] * 5;
            inputNodes[(i / 2) + inputBrick->nodePos] = (static_cast<float>(total)) + 255.0f;
        }

        /*for(uint32_t x = 0; x < 28; x++)
        {
            for(uint32_t y = 0; y < 28; y++)
            {
                const uint32_t pos = x * 28 + y + pic * pictureSize + 16;
                if(dataBufferPtr[pos] > 150) {
                    std::cout<<" x";
                } else {
                    std::cout<<"  ";
                }
            }
            std::cout<<std::endl;
        }*/


        uint16_t tryCount = 5;
        bool result = false;
        while(tryCount > 0
              && result == false)
        {
            result = KyoukoRoot::m_root->learnStep();
            tryCount--;
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
    uint32_t total = 100;
    for(uint32_t pic = 0; pic < total; pic++)
    {
        //std::cout<<pic<<" should: "<<(int)labelBufferPtr[pic + 8]<<"   is: ";
        std::cout<<pic<<" should: "<<(int)labelBufferPtr[pic + 8]<<"   is: ";

        Brick* inputBrick = KyoukoRoot::m_segment->inputBricks[0];
        float* inputNodes = Kitsunemimi::getBuffer<float>(KyoukoRoot::m_segment->nodeInputBuffer);
        for(uint32_t i = 0; i < pictureSize; i = i + 2)
        {
            const uint32_t pos = pic * pictureSize + i + 16;
            uint32_t total = dataBufferPtr[pos] * 5 + dataBufferPtr[pos + 1] * 5;
            inputNodes[(i / 2) + inputBrick->nodePos] = (static_cast<float>(total)) + 255.0f;
        }

        executeStep();

        // print result
        float biggest = 0.0;
        uint32_t pos = 0;
        Output* outputs = Kitsunemimi::getBuffer<Output>(KyoukoRoot::m_segment->outputs);
        std::string outString = "[";
        for(uint32_t i = 0; i < KyoukoRoot::m_segment->outputs.numberOfItems; i++)
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

        if(labelBufferPtr[pic + 8] == pos) {
            match++;
        }
    }

    std::cout<<"======================================================================="<<std::endl;
    std::cout<<"corrct: "<<match<<"/"<<total<<std::endl;
    std::cout<<"======================================================================="<<std::endl;
}

