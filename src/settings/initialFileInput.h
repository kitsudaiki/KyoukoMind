#ifndef INITIALFILEINPUT_H
#define INITIALFILEINPUT_H

#include <QString>
#include <QFile>
#include <QByteArray>
#include <QMap>
#include <QPair>

namespace KyoukoMind
{

class ClusterHandler;

class InitialFileInput
{
public:
    InitialFileInput();

    bool readInitialFile(const QString filePath,
                         ClusterHandler *clusterManager,
                         const QString directoryPath);

};

}

#endif // INITIALFILEINPUT_H
