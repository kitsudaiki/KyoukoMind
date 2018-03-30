#include <core/networkManager.h>
#include <settings/config.h>
#include <KyoChanNetwork.h>

#include <core/cluster/clusterHandler.h>
#include <core/processing/processingUnitHandler.h>
#include <core/messaging/messagecontroller.h>

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
    m_messageController = new MessageController();

    bool ok = false;
    std::string initialFile = KyoukoNetwork::m_config->getInitialFilePath(&ok);
    std::string directoryPath = KyoukoNetwork::m_config->getDirectoryPath(&ok);

    std::vector<std::string> clusterFiles;
    // get all files in the directory
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir(directoryPath.c_str())) != nullptr) {
        while((ent = readdir(dir)) != nullptr) {
            clusterFiles.push_back(ent->d_name);
        }
        closedir (dir);
    }

    if(clusterFiles.size() == 0)
    {
        bool successfulInit = readInitialFile(initialFile,
                                              directoryPath,
                                              m_clusterManager);
        assert(successfulInit);
    }
    else {
        for(uint32_t i = 0; i < clusterFiles.size(); i++) {
            // TODO
        }
    }
}

/**
 * @brief NetworkManager::getMindDimension
 * @return
 */
uint32_t *NetworkManager::getMindDimension()
{
    return m_mindDimensions;
}

/**
 * @brief NetworkManager::splitString
 * @param s
 * @param delim
 * @return
 */
std::vector<std::string> NetworkManager::splitString(const std::string &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while(std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

/**
 * @brief NetworkManager::removeEmptyStrings
 * @param strings
 */
void NetworkManager::removeEmptyStrings(std::vector<std::string>& strings)
{
    std::vector<std::string>::iterator it = std::remove_if(
              strings.begin(),
              strings.end(),
              std::mem_fun_ref(&std::string::empty));
    // erase the removed elements
    strings.erase(it, strings.end());
}

/**
 * @brief NetworkManager::readInitialFile
 * @param filePath
 * @param directoryPath
 * @param clusterManager
 * @return
 */
bool NetworkManager::readInitialFile(const std::string filePath,
                                     const std::string directoryPath,
                                     ClusterHandler* clusterManager)
{
    ClusterID idCounter = 0;
    bool ok = false;
    uint32_t nodeNumberPerCluster = KyoukoNetwork::m_config->getNumberOfNodes(&ok);

    // read into string
    std::ifstream inFile;
    inFile.open(filePath);
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::string string_content = strStream.str();

    // erase whitespaces
    string_content.erase(std::remove_if(string_content.begin(),
                                        string_content.end(),
                                        isspace),
                         string_content.end());

    // split string
    std::vector<std::string> allLines = splitString(string_content, '\n');

    // read the single lines
    for(unsigned int lineNumber = 0; lineNumber < allLines.size(); lineNumber++)
    {
        // split line
        std::vector<std::string> splittedLine = splitString(allLines[lineNumber], '|');

        // remove empty entries from the list
        removeEmptyStrings(splittedLine);

        // process the splitted line
        for(unsigned int linePartNumber = 0; linePartNumber < splittedLine.size(); linePartNumber++)
        {
            int number = std::stoi(splittedLine[linePartNumber]);

            Cluster* cluster = nullptr;
            // create cluster
            switch (number) {
                case 0:
                    cluster = new EmptyCluster(idCounter, directoryPath, m_messageController);
                    break;
                case 1:
                    cluster = new EdgeCluster(idCounter, directoryPath, m_messageController);
                    break;
                case 2:
                    cluster = new NodeCluster(idCounter, directoryPath, nodeNumberPerCluster, m_messageController);
                    break;
                default:
                    return false;
            }
            clusterManager->addCluster(idCounter, cluster);
            idCounter++;
        }
    }
    return true;
}

}
