#include <QCoreApplication>
#include <src/KyoChanNetwork.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    KyoukoMind::KyoChanNetwork* network = new KyoukoMind::KyoChanNetwork("../../config.ini");

    return a.exec();
}
