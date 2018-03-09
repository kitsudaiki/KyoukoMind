#ifndef NETTHREADMANAGER_H
#define NETTHREADMANAGER_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QMap>
#include <QDir>
#include <QPair>

#include <common/typedefs.h>

namespace KyoukoMind
{

class Config;
class ClusterHandler;
class ProcessingUnitHandler;

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    NetworkManager();
    void startLoops();
    bool setThread(QThread *thread);

    quint32* getMindDimension();

signals:
    void processNetwork_Signal();

private:
    ClusterHandler* m_clusterManager = nullptr;
    ProcessingUnitHandler* m_processingUnitHandler = nullptr;

    quint32 m_numberOfThreads = 0;
    QThread *m_thread = nullptr;

    quint32 m_mindDimensions[3];

    bool readInitialFile(const QString filePath,
                         const QString directoryPath,
                         ClusterHandler *clusterManager);
};
}

#endif // MAINLOOP_H
