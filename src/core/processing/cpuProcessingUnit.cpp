#include <core/processing/cpuProcessingUnit.h>

namespace KyoukoMind
{

CpuProcessingUnit::CpuProcessingUnit()
{

}

void CpuProcessingUnit::processCluster(Cluster *cluster)
{
    /*for(int i = m_startPoint; i < m_startPoint+m_offset; i++)
    {
        KyoChanNode* tempNode = (KyoChanNode*)&m_nodeNetwork->at(i);
        uint32_t pos = 0;
        qint32 currentState = tempNode->state;
        tempNode->state /= 3;

        if(currentState > tempNode->border)
        {
            QVectorIterator<KyoChanEdge> i(tempNode->edges);
            while(i.hasNext())
            {
                KyoChanEdge currentEdge = i.next();
                pos = currentEdge.target;
                (*m_nodeNetwork)[pos].tempStates[m_id] += currentEdge.weight;

            }
        }
    }*/
}

}
