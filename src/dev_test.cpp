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
#include <core/orchestration/network_cluster.h>
#include <core/orchestration/segments/input_segment.h>
#include <core/orchestration/segments/output_segment.h>

#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCommon/progress_bar.h>
#include <libKitsunemimiCrypto/common.h>

#include <core/processing/cpu/cpu_processing_unit.h>
#include <core/processing/gpu/gpu_processing_uint.h>

#include <core/orchestration/cluster_handler.h>
#include <core/orchestration/cluster_interface.h>

#include <core/orchestration/task_queue.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

/**
 * @brief only a test-function for fast tests
 *
 * @param mnistRootPath absolute path to the directory with the MNIST test-files
 */
void
learnTestData(const std::string &mnistRootPath)
{
    SakuraLangInterface* iface = SakuraLangInterface::getInstance();
    DataMap result;
    DataMap context;
    Kitsunemimi::Sakura::BlossomStatus status;
    Kitsunemimi::ErrorContainer error;

    //----------------------------------------------------------------------------------------------

    DataMap deleteClusterValues2;
    deleteClusterValues2.insert("name", new DataValue("test_cluster"));
    iface->triggerBlossom(result,
                          "delete",
                          "cluster",
                          context,
                          deleteClusterValues2,
                          status,
                          error);

    DataMap deleteTemplateValues2;
    deleteTemplateValues2.insert("name", new DataValue("test_template"));
    iface->triggerBlossom(result,
                          "delete",
                          "template",
                          context,
                          deleteTemplateValues2,
                          status,
                          error);

    //----------------------------------------------------------------------------------------------

    DataMap templateCreateValues;
    templateCreateValues.insert("name", new DataValue("test_template"));
    templateCreateValues.insert("number_of_inputs", new DataValue(784));
    templateCreateValues.insert("number_of_outputs", new DataValue(10));
    iface->triggerBlossom(result,
                          "create",
                          "template",
                          context,
                          templateCreateValues,
                          status,
                          error);
    std::cout<<"result create template: "<<result.toString()<<std::endl;

    const std::string templateUuid = result.getStringByKey("uuid");
    std::cout<<"template uuid: "<<templateUuid<<std::endl;

    result.clear();
    error._errorMessages.clear();

    //----------------------------------------------------------------------------------------------

    DataMap clusterCreateValues;
    clusterCreateValues.insert("name", new DataValue("test_cluster"));
    clusterCreateValues.insert("template_uuid", new DataValue(templateUuid));
    iface->triggerBlossom(result,
                          "create",
                          "cluster",
                          context,
                          clusterCreateValues,
                          status,
                          error);
    std::cout<<"result create: "<<result.toString()<<std::endl;

    const std::string clusterUuid = result.getStringByKey("uuid");
    std::cout<<"cluster uuid: "<<templateUuid<<std::endl;

    result.clear();
    error._errorMessages.clear();

    //----------------------------------------------------------------------------------------------

    ClusterInterface* clusterInterface = KyoukoRoot::m_clusterHandler->getCluster(clusterUuid);

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
    Kitsunemimi::BinaryFile trainData(trainDataPath);
    Kitsunemimi::DataBuffer trainDataBuffer;
    trainData.readCompleteFile(trainDataBuffer);

    // read train-labels
    Kitsunemimi::BinaryFile trainLabel(trainLabelPath);
    Kitsunemimi::DataBuffer trainLabelBuffer;
    trainLabel.readCompleteFile(trainLabelBuffer);

    std::cout<<trainDataBuffer.usedBufferSize<<std::endl;
    std::cout<<trainLabelBuffer.usedBufferSize<<std::endl;

    std::string inputData;
    Kitsunemimi::Crypto::encodeBase64(inputData, trainDataBuffer.data, trainDataBuffer.usedBufferSize);
    std::string labelData;
    Kitsunemimi::Crypto::encodeBase64(labelData, trainLabelBuffer.data, trainLabelBuffer.usedBufferSize);

    //----------------------------------------------------------------------------------------------

    for(uint32_t learnRun = 0; learnRun < 5; learnRun++)
    {
        DataMap taskLearnValues;
        taskLearnValues.insert("cluster_uuid", new DataValue(clusterUuid));
        taskLearnValues.insert("input_data_uuid", new DataValue(""));
        taskLearnValues.insert("label_data_uuid", new DataValue(""));
        taskLearnValues.insert("type", new DataValue("mnist"));
        taskLearnValues.insert("input_data", new DataValue(inputData));
        taskLearnValues.insert("label_data", new DataValue(labelData));
        iface->triggerBlossom(result,
                              "create_learn",
                              "task",
                              context,
                              taskLearnValues,
                              status,
                              error);
        std::cout<<"result create: "<<result.toString()<<std::endl;

        const std::string learnTaskUuid = result.getStringByKey("uuid");
        std::cout<<"cluster uuid: "<<templateUuid<<std::endl;

        result.clear();
        error._errorMessages.clear();


        // wait until task is finished
        start = std::chrono::system_clock::now();
        Kitsunemimi::ProgressBar progressBar;
        while(clusterInterface->isFinish(learnTaskUuid) == false)
        {
            const TaskProgress progress = clusterInterface->getProgress(learnTaskUuid);
            progressBar.updateProgress(progress.percentageFinished);
            usleep(100000);
        }
        progressBar.updateProgress(1.0f);
        end = std::chrono::system_clock::now();
        const float time = std::chrono::duration_cast<chronoSec>(end - start).count();

        std::cout<<"run learn: "<<time<<"s"<<std::endl;
    }

    //==============================================================================================
    // test
    //==============================================================================================

    // read train-data
    Kitsunemimi::BinaryFile testData(testDataPath);
    Kitsunemimi::DataBuffer testDataBuffer;
    testData.readCompleteFile(testDataBuffer);

    // read train-labels
    Kitsunemimi::BinaryFile testLabel(testLabelPath);
    Kitsunemimi::DataBuffer testLabelBuffer;
    testLabel.readCompleteFile(testLabelBuffer);


    uint8_t* testDataBufferPtr = static_cast<uint8_t*>(testDataBuffer.data);
    uint8_t* testLabelBufferPtr = static_cast<uint8_t*>(testLabelBuffer.data);

    std::string requestInputData;
    Kitsunemimi::Crypto::encodeBase64(requestInputData, testDataBuffer.data, testDataBuffer.usedBufferSize);
    std::string  requestLabelData;
    Kitsunemimi::Crypto::encodeBase64(requestLabelData, testLabelBuffer.data, testLabelBuffer.usedBufferSize);

    //----------------------------------------------------------------------------------------------

    DataMap taskRequestValues;
    taskRequestValues.insert("cluster_uuid", new DataValue(clusterUuid));
    taskRequestValues.insert("input_data_uuid", new DataValue(""));
    taskRequestValues.insert("type", new DataValue("mnist"));
    taskRequestValues.insert("input_data", new DataValue(requestInputData));
    iface->triggerBlossom(result,
                          "create_request",
                          "task",
                          context,
                          taskRequestValues,
                          status,
                          error);
    std::cout<<"result create: "<<result.toString()<<std::endl;

    const std::string requestTaskUuid = result.getStringByKey("uuid");
    std::cout<<"cluster uuid: "<<templateUuid<<std::endl;

    result.clear();
    error._errorMessages.clear();


    // wait until task is finished
    start = std::chrono::system_clock::now();
    Kitsunemimi::ProgressBar progressBar2;
    while(clusterInterface->isFinish(requestTaskUuid) == false)
    {
        const TaskProgress progress = clusterInterface->getProgress(requestTaskUuid);
        progressBar2.updateProgress(progress.percentageFinished);
        usleep(100000);
    }
    progressBar2.updateProgress(1.0f);
    end = std::chrono::system_clock::now();
    const float time2 = std::chrono::duration_cast<chronoSec>(end - start).count();

    std::cout<<"run learn: "<<time2<<"s"<<std::endl;

    //----------------------------------------------------------------------------------------------

    uint32_t match = 0;
    uint32_t total = 10000;
    const uint32_t* resultData = clusterInterface->getResultData(requestTaskUuid);

    for(uint32_t pic = 0; pic < total; pic++)
    {
        const uint32_t label = testLabelBufferPtr[pic + 8];
        if(resultData[pic] == label) {
            match++;
        }
    }

    std::cout<<"======================================================================="<<std::endl;
    std::cout<<"correct: "<<match<<"/"<<total<<std::endl;
    std::cout<<"======================================================================="<<std::endl;

    //----------------------------------------------------------------------------------------------
}

