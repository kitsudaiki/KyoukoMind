#include <QCoreApplication>
#include <src/KyoChanNetwork.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    KyoukoMind::KyoukoNetwork* network = new KyoukoMind::KyoukoNetwork("../../KyoukoMind/config.ini");
    return a.exec();
}
