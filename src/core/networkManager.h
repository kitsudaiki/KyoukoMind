#ifndef NETTHREADMANAGER_H
#define NETTHREADMANAGER_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <common/typedefs.h>

namespace KyoukoMind
{

class NetIO;
class Config;
class ClusterHandler;

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    NetworkManager();
    void startLoops();
    void initNodeNetwork();

    bool setThread(QThread *thread);

signals:
    void processNetwork_Signal();

private:
    ClusterHandler* m_clusterManager = nullptr;
    quint32 m_numberOfThreads = 0;
    QThread *m_thread = nullptr;
    NetIO *m_netIO = nullptr;
};
}

#endif // MAINLOOP_H
