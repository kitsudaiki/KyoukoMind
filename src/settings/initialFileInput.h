#ifndef INITIALFILEINPUT_H
#define INITIALFILEINPUT_H

#include <QString>
#include <QFile>
#include <QByteArray>
#include <QMap>
#include <QPair>

namespace KyoukoMind
{

class ClusterManager;

class InitialFileInput
{
public:
    InitialFileInput();

    bool readInitialFile(QString filePath, ClusterManager *clusterManager);

};

}

#endif // INITIALFILEINPUT_H
