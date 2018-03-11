#ifndef CLUSTERTEST_H
#define CLUSTERTEST_H

#include <QObject>
#include <QString>
#include <QtTest>
#include <QFile>


namespace KyoukoMind
{

class NodeCluster;
class EdgeCluster;
class EmptyCluster;

class ClusterTest : public QObject
{
    Q_OBJECT

public:
    ClusterTest();

private slots:
    void initTestCase();
    void checkNodeCluster();
    void checkEdgeCluster();
    void checkEmptyCluster();
    void cleanupTestCase();

private:
    NodeCluster *m_noteCluster = nullptr;
    EdgeCluster *m_edgeCluster = nullptr;
    EmptyCluster *m_emptyCluster = nullptr;
};

}

#endif // CLUSTERTEST_H
