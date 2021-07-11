#include "dev_test.h"

#include <kyouko_root.h>
#include <core/objects/network_cluster.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/text_file.h>

#include <core/processing/cpu_processing_unit.h>
#include <core/processing/gpu/gpu_processing_uint.h>

void
learnTestData(const std::string &mnistRootPath)
{
    NetworkCluster* cluster = KyoukoRoot::m_networkCluster;
    CpuProcessingUnit cpuProcessingUnit;

    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = new Kitsunemimi::Opencl::GpuHandler();
    GpuProcessingUnit gpuProcessingUnit(m_gpuHandler->m_interfaces.at(0));
    assert(gpuProcessingUnit.initializeGpu(cluster));

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
    InputNode* inputNodes = cluster->synapseSegment->inputs;
    for(uint32_t i = 0; i < 784; i++)  {
        inputNodes[i].weight = 0.0f;
    }

    std::cout<<"learn"<<std::endl;

    for(uint32_t poi = 0; poi < 1; poi++)
    {
        for(uint32_t pic = 0; pic < 1000; pic++)
        {
            const uint32_t label = labelBufferPtr[pic + 8];
            std::cout<<"picture: "<<pic<<std::endl;

            OutputNode* outputs = cluster->synapseSegment->outputs;
            for(uint32_t i = 0; i < 10; i++) {
                outputs[i].shouldValue = 0.0f;
            }

            outputs[label].shouldValue = 1.0f;
            std::cout<<"label: "<<label<<std::endl;

            for(uint32_t i = 0; i < pictureSize; i++)
            {
                const uint32_t pos = pic * pictureSize + i + 16;
                int32_t total = dataBufferPtr[pos];
                inputNodes[i].weight = (static_cast<float>(total) / 255.0f);
            }

            //cpuProcessingUnit.learn();
            gpuProcessingUnit.learn();
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

    for(uint32_t i = 0; i < 784; i++)  {
        inputNodes[i].weight = 0.0f;
    }

    Segment* synapseSegment = KyoukoRoot::m_networkCluster->synapseSegment;

    for(uint32_t pic = 0; pic < total; pic++)
    {
        uint32_t label = testLabelBufferPtr[pic + 8];

        std::cout<<pic<<" should: "<<label<<"   is: ";

        for(uint32_t i = 0; i < pictureSize; i++)
        {
            const uint32_t pos = pic * pictureSize + i + 16;
            int32_t total = testDataBufferPtr[pos];
            inputNodes[i].weight = (static_cast<float>(total) / 255.0f);
        }

        //cpuProcessingUnit.execute();
        gpuProcessingUnit.execute();

        // print result
        float biggest = -100000.0f;
        uint32_t pos = 0;
        std::cout<<"[";


        for(uint64_t i = 0; i < synapseSegment->segmentHeader->outputs.count; i++)
        {
            OutputNode* out = &synapseSegment->outputs[i];

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
}

