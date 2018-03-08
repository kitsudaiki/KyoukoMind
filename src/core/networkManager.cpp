#include <core/networkManager.h>
#include <settings/config.h>
#include <KyoChanNetwork.h>

#include <core/cluster/clusterHandler.h>
#include <core/processing/processingUnitHandler.h>

#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>

namespace KyoukoMind
{

/**
 * @brief NetManager::NetManager
 */
NetworkManager::NetworkManager()
{
    m_clusterManager = new ClusterHandler();
    m_processingUnitHandler = new ProcessingUnitHandler();
}

/**
 * @brief NetManager::startLoops
 */
void NetworkManager::startLoops()
{
    for(unsigned int i = 0; i < m_numberOfThreads; i++)
    {
        //NetThread* netThread = new NetThread(i);
        //m_netThreads.push_back(netThread);
        //netThread->startThread();
    }

    /*m_loopEnd = chronoClock::now();
    long timeSpan = std::chrono::duration_cast<chronoMicroSec>(m_loopEnd-m_loopStart).count();
    long timeDiff = (1000000 / LOOP_RUNS_PER_SECOND) - timeSpan;
    if(timeDiff > 0)
    {
        std::this_thread::sleep_for(chronoMicroSec(timeDiff));
    }*/
}

/**
 * @brief NetManager::setThread
 * @param thread
 * @return
 */
bool NetworkManager::setThread(QThread *thread)
{
    if(m_thread == nullptr)
    {
        m_thread = thread;
        return true;
    }
    return false;
}

/**
 * @brief NetworkManager::getMindDimension
 * @return
 */
quint32 *NetworkManager::getMindDimension()
{
    return m_mindDimensions;
}

/**
 * @brief NetworkManager::readInitialFile
 * @param filePath
 * @param clusterManager
 * @return
 */
bool NetworkManager::readInitialFile(const QString filePath,
                                     ClusterHandler* clusterManager,
                                     const QString directoryPath)
{
    bool ok = false;
    quint32 nodeNumberPerCluster = KyoukoNetwork::m_config->getNumberOfNodes(&ok);

    QFile initialFile(filePath);
    if(!initialFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray content = initialFile.readAll();
    QString string_content(content);
    string_content = string_content.replace(" ", "");
    QStringList allLines = string_content.split('\n');

    for(int lineNumber = 0; lineNumber < allLines.size(); lineNumber++)
    {
        QStringList splittedLine = allLines[lineNumber].split('|');
        for(int linePartNumber = 0; linePartNumber < splittedLine.size(); linePartNumber++)
        {
            if(splittedLine.at(linePartNumber).isEmpty()) {
                splittedLine.removeAt(linePartNumber);
                linePartNumber--;
            }
        }
        for(int linePartNumber = 0; linePartNumber < splittedLine.size(); linePartNumber++)
        {
            int number = splittedLine[linePartNumber].toInt();

            ClusterID clusterId;
            clusterId.x = lineNumber;
            clusterId.y = linePartNumber;
            // TODO: add z-dimension
            clusterId.z = 0;

            Cluster* cluster = nullptr;
            switch (number) {
            case 0:
                cluster = new EmptyCluster(clusterId, directoryPath);
                break;
            case 1:
                cluster = new EdgeCluster(clusterId, directoryPath);
                break;
            case 2:
                cluster = new NodeCluster(clusterId, directoryPath, nodeNumberPerCluster);
                break;
            default:
                cluster = new EmptyCluster(clusterId, directoryPath);
                break;
            }
            clusterManager->addCluster(clusterId, cluster);
        }
    }
    return true;
}

}
