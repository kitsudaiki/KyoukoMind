#ifndef CLUSTERTEST_H
#define CLUSTERTEST_H

#include <QObject>
#include <QString>
#include <QtTest>
#include <QFile>
#include <core/cluster/nodeCluster.h>

namespace KyoChan_Network
{

class ClusterTest : public QObject
{
    Q_OBJECT

public:
    ClusterTest();

private slots:
    void initTestCase();
    void checkSizeAfterInit();
    void cleanupTestCase();

private:
    NodeCluster *m_cluster = nullptr;
};

}

#endif // CLUSTERTEST_H
