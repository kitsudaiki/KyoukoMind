#include <QCoreApplication>
#include <src/KyoChanNetwork.h>
#include <tests/test.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //KyoukoMind::KyoukoNetwork* network = new KyoukoMind::KyoukoNetwork("../../config.ini");
    Test testRun;
    testRun.runClusterTests(argc, argv);

    //return a.exec();
}
