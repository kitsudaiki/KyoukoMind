#include <src/KyoChanNetwork.h>
#include <tests/clustertest.h>
#include <tests/messagetest.h>

int main(int argc, char *argv[])
{
    //KyoukoMind::KyoukoNetwork* network = new KyoukoMind::KyoukoNetwork("../../KyoukoMind/config.ini");
    KyoukoMind::ClusterTest();
    KyoukoMind::MessageTest();
}
