#include <settings/initialFileInput.h>
#include <core/cluster/clusterHandler.h>
#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>
#include <KyoChanNetwork.h>

namespace KyoukoMind
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
bool InitialFileInput::readInitialFile(const QString filePath,
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
