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
#include <tests/processingTest.h>
#include <tests/storageMemoryTest.h>
#include <tests/dataBufferTest.h>
#include <tests/structTest.h>

#include <tests/proofOfConcept/demoUserInput.h>

int main(int argc, char *argv[])
{

    KyoukoMind::DataBufferTest();
    KyoukoMind::StorageMemoryTest();
    KyoukoMind::InitTest();
    KyoukoMind::ClusterTest();
    KyoukoMind::MessageTest();
    KyoukoMind::StructTest();
    KyoukoMind::ProcessingTest();


    KyoukoMind::KyoukoNetwork* network = new KyoukoMind::KyoukoNetwork("/home/neptune/Schreibtisch/Projekte/KyoukoMind/config.ini");

    KyoukoMind::DemoUserInput input(network->getClusterHandler());
    input.inputLoop();
}
