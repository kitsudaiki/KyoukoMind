/**
 *  @file    main.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <src/kyochanNetwork.h>
#include <tests/clusterTest.h>
#include <tests/messageTest.h>
#include <tests/initTest.h>
#include <tests/messageBufferTest.h>
#include <tests/processingTest.h>
#include <tests/storageMemoryTest.h>
#include <tests/dataBufferTest.h>
#include <tests/structTest.h>

#include <tests/proofOfConcept/demoUserInput.h>

int main(int argc, char *argv[])
{

    /*KyoukoMind::DataBufferTest();
    KyoukoMind::StorageMemoryTest();
    KyoukoMind::InitTest();
    KyoukoMind::ClusterTest();
    KyoukoMind::MessageTest();
    KyoukoMind::StructTest();
    KyoukoMind::MessageBufferTest();
    KyoukoMind::ProcessingTest();*/


    KyoukoMind::KyoukoNetwork* network = new KyoukoMind::KyoukoNetwork("../../KyoukoMind/config.ini");

    KyoukoMind::DemoUserInput input(network->getMessageController(),
                                    network->getClusterHandler());
    input.inputLoop();
}
