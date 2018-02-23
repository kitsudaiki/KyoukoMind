#include <QCoreApplication>
#include <src/KyoChanNetwork.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    KyoChan_Network::KyoChanNetwork* network = new KyoChan_Network::KyoChanNetwork("../../config.ini");

    return a.exec();
}
