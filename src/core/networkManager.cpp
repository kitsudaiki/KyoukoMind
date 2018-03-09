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

    bool ok = false;
    QString initialFile = KyoukoNetwork::m_config->getInitialFilePath(&ok);
    QString directoryPath = KyoukoNetwork::m_config->getDirectoryPath(&ok);

    QFileInfoList clusterFiles = QDir(directoryPath).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries);
    if(clusterFiles.count() == 0)
    {
        KyoukoNetwork::m_logger->logDebug("init new cluster from file " + initialFile);
        bool successfulInit = readInitialFile(initialFile,
                                              directoryPath,
                                              m_clusterManager);
        if(successfulInit) {
            KyoukoNetwork::m_logger->logError("initial File is invalid");
        }
        assert(successfulInit);
    }
    else {
        for(quint32 i = 0; i < clusterFiles.length(); i++) {
            QFileInfo info = clusterFiles.at(i);
            // TODO
        }
    }
}

/**
 * @brief NetManager::startLoops
 */
void NetworkManager::startLoops()
{

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
 * @param directoryPath
 * @param clusterManager
 * @return
 */
bool NetworkManager::readInitialFile(const QString filePath,
                                     const QString directoryPath,
                                     ClusterHandler* clusterManager)
{
    bool ok = false;
    quint32 nodeNumberPerCluster = KyoukoNetwork::m_config->getNumberOfNodes(&ok);

    // read and split the file
    QFile initialFile(filePath);
    if(!initialFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray content = initialFile.readAll();
    QString string_content(content);
    string_content = string_content.replace(" ", "");
    QStringList allLines = string_content.split('\n');

    // read the single lines
    for(int lineNumber = 0; lineNumber < allLines.size(); lineNumber++)
    {
        // split line
        QStringList splittedLine = allLines[lineNumber].split('|');

        // remove empty entries from the list
        for(int linePartNumber = 0; linePartNumber < splittedLine.size(); linePartNumber++)
        {
            if(splittedLine.at(linePartNumber).isEmpty()) {
                splittedLine.removeAt(linePartNumber);
                linePartNumber--;
            }
        }
        // process the splitted line
        for(int linePartNumber = 0; linePartNumber < splittedLine.size(); linePartNumber++)
        {
            int number = splittedLine[linePartNumber].toInt();

            ClusterID clusterId;
            clusterId.x = lineNumber;
            clusterId.y = linePartNumber;
            // TODO: add z-dimension
            clusterId.z = 0;

            Cluster* cluster = nullptr;
            // create cluster
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
                    return false;
            }
            clusterManager->addCluster(clusterId, cluster);
        }
    }
    return true;
}

}
