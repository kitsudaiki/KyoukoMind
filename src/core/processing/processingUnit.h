#ifndef PROCESSINGUNIT_H
#define PROCESSINGUNIT_H

#include <QObject>

namespace KyoukoMind
{
class Cluster;
class ClusterHandler;

class ProcessingUnit : public QObject
{
    Q_OBJECT

public:
    ProcessingUnit();

    virtual void processCluster(Cluster* cluster) = 0;
};

}

#endif // PROCESSINGUNIT_H
