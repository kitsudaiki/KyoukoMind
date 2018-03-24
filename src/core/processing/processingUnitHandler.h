#ifndef PROCESSINGUNITHANDLER_H
#define PROCESSINGUNITHANDLER_H

#include <QVector>

namespace KyoukoMind
{
class ProcessingUnit;

class ProcessingUnitHandler
{
public:
    ProcessingUnitHandler();

private:
    QVector<ProcessingUnit*> m_allProcessingUnits;
};

}

#endif // PROCESSINGUNITHANDLER_H
