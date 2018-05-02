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
#include <tests/messageBufferTest.h>
#include <tests/processingtest.h>
#include <tests/storageMemoryTest.h>
#include <tests/dataBufferTest.h>

#include <tests/proofOfConcept/demoUserInput.h>

int main(int argc, char *argv[])
{
    //KyoukoMind::KyoukoNetwork* network = new KyoukoMind::KyoukoNetwork("../../KyoukoMind/config.ini");
    KyoukoMind::ClusterTest();
    //KyoukoMind::MessageTest();
    KyoukoMind::MessageBufferTest();
    KyoukoMind::ProcessingTest();
    //KyoukoMind::DemoUserInput input(network->getMessageController());
    //input.inputLoop();
}
