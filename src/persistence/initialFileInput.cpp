#include <persistence/initialFileInput.h>
#include <core/cluster/clusterHandler.h>
#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>
#include <KyoChanNetwork.h>

namespace KyoChan_Network
{

InitialFileInput::InitialFileInput()
{

}

/**
 * @brief InitialFileInput::readInitialFile
 * @param filePath
 * @param clusterManager
 * @return
 */
bool InitialFileInput::readInitialFile(QString filePath, ClusterManager* clusterManager)
{
    quint32 clusterId = 0;
    bool ok = false;
    quint32 nodeNumberPerCluster = KyoChanNetwork::m_config->getNumberOfNodes(&ok);

    QMap<QPair<int, int>, quint32> m_clusterStructure;

    QFile initialFile(filePath);
    QByteArray content = initialFile.readAll();
    content = content.replace(" ", "");
    QList<QByteArray> allLines = content.split('\n');

    for(int i = 0; i < allLines.size(); i++)
    {
        QList<QByteArray> splittedLine = allLines[i].split('|');
        for(int j = 0; j < splittedLine.size(); j++)
        {
            int number = splittedLine[j].toInt();
            Cluster* cluster = nullptr;
            switch (number) {
            case 0:
                cluster = new EmptyCluster(clusterId);
                break;
            case 1:
                cluster = new EdgeCluster(clusterId);
                break;
            case 2:
                cluster = new NodeCluster(clusterId, nodeNumberPerCluster);
                break;
            default:
                cluster = new EmptyCluster(clusterId);
                break;
            }
            m_clusterStructure.insert(qMakePair(i,j), clusterId);
            clusterManager->addCluster(clusterId, cluster);
            clusterId++;
        }
    }

    //TODO: schönere Lösung finden
    for(int i = 0; i < allLines.size(); i++)
    {
        QList<QByteArray> splittedLine = allLines[i].split('|');
        for(int j = 0; j < splittedLine.size(); j++)
        {
            for(int z = 0; z < 6; z++)
            {
                QPair<int, int> pos = qMakePair(i,j);
            }

            m_clusterStructure.insert(qMakePair(i,j), clusterId);
            //clusterManager->addCluster(clusterId, cluster);
            clusterId++;
        }
    }
    return true;
}

}
