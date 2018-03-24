#ifndef NETTHREADMANAGER_H
#define NETTHREADMANAGER_H

#include <QThread>
#include <QVector>
#include <string>
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

class NetworkManager
{

public:
    NetworkManager();
    void startLoops();
    bool setThread(QThread *thread);

    uint32_t* getMindDimension();

signals:
    void processNetwork_Signal();

private:
    ClusterHandler* m_clusterManager = nullptr;
    ProcessingUnitHandler* m_processingUnitHandler = nullptr;

    uint32_t m_numberOfThreads = 0;
    QThread *m_thread = nullptr;

    uint32_t m_mindDimensions[3];

    bool readInitialFile(const std::string filePath,
                         const std::string directoryPath,
                         ClusterHandler *clusterManager);

    std::vector<std::string> splitString(const std::string &s, char delim);
    void removeEmptyStrings(std::vector<std::string> &strings);
};
}

#endif // MAINLOOP_H
