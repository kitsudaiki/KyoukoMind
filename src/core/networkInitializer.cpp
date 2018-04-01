#include "networkInitializer.h"
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
 * @brief NetworkInitializer::NetworkInitializer
 * @param filePath
 * @param directoryPath
 * @param clusterManager
 * @param messageController
 */
NetworkInitializer::NetworkInitializer(const std::string filePath,
                                       const std::string directoryPath,
                                       ClusterHandler *clusterManager,
                                       MessageController *messageController)
{
    m_filePath = filePath;
    m_directoryPath = directoryPath;
    m_clusterManager = clusterManager;
    m_messageController = messageController;
}

/**
 * @brief NetworkInitializer::initNetwork
 * @return
 */
bool NetworkInitializer::initNetwork()
{
    // check if values are valid
    if(m_clusterManager == nullptr
            || m_filePath == ""
            || m_directoryPath == "")
    {
        return false;
    }

    // check if initial file exist
    std::ifstream infile(m_filePath);
    if(!infile) {
        return false;
    }

    m_idCounter = 1;

    if(getNetworkMetaStructure() == false) {
        return false;
    }

    if(createNetwork() == false) {
        return false;
    }

    if(createAxons() == false) {
        return false;
    }

    return true;
}

/**
 * @brief NetworkInitializer::getNetworkMetaStructure
 * @return
 */
bool NetworkInitializer::getNetworkMetaStructure()
{
    uint32_t firstLineLenght = 0;

    // read into string
    std::ifstream inFile;
    inFile.open(m_filePath);
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
    for(uint32_t lineNumber = 0; lineNumber < allLines.size(); lineNumber++)
    {
        // split line
        std::vector<std::string> splittedLine = splitString(allLines[lineNumber], '|');

        // remove empty entries from the list
        removeEmptyStrings(splittedLine);

        // add new line to meat-structure-vector
        std::vector<std::pair<uint8_t, ClusterID>> newLine;
        m_networkMetaStructure.push_back(newLine);

        // process the splitted line
        for(uint32_t linePartNumber = 0; linePartNumber < splittedLine.size(); linePartNumber++)
        {
            if(linePartNumber == 0) {
                firstLineLenght = splittedLine.size();
            } else {
                if(firstLineLenght != splittedLine.size()) {
                    // TODO: exception-message
                    return false;
                }
            }
            uint8_t number = std::stoi(splittedLine[linePartNumber]);
            m_networkMetaStructure[lineNumber].push_back(std::make_pair(number,m_idCounter));
            m_idCounter++;
        }
    }

    // save network-dimensions
    m_networkDimensions[0] = allLines.size();
    m_networkDimensions[1] = firstLineLenght;

    return true;
}

/**
 * @brief NetworkInitializer::createNetwork
 * @return
 */
bool NetworkInitializer::createNetwork()
{
    bool ok = false;
    uint32_t nodeNumberPerCluster = KyoukoNetwork::m_config->getNumberOfNodes(&ok);

    for(uint32_t x = 0; x < m_networkMetaStructure.size(); x++)
    {
        for(uint32_t y = 0; y < m_networkMetaStructure[x].size(); y++)
        {
            addCluster(x, y, nodeNumberPerCluster);
        }
    }
    return true;
}

/**
 * @brief NetworkInitializer::addCluster
 * @param x
 * @param y
 * @param nodeNumberPerCluster
 * @return
 */
bool NetworkInitializer::addCluster(const uint32_t x,
                                    const uint32_t y,
                                    const uint32_t nodeNumberPerCluster)
{
    Cluster* cluster = nullptr;
    // create cluster
    switch ((int)m_networkMetaStructure[x][y].first) {
        case 0:
            cluster = new EmptyCluster(m_networkMetaStructure[x][y].second,
                                       m_directoryPath,
                                       m_messageController);
            break;
        case 1:
            cluster = new EdgeCluster(m_networkMetaStructure[x][y].second,
                                      m_directoryPath,
                                      m_messageController);
            break;
        case 2:
            cluster = new NodeCluster(m_networkMetaStructure[x][y].second,
                                      m_directoryPath,
                                      nodeNumberPerCluster,
                                      m_messageController);
            break;
        default:
            return false;
    }
    m_clusterManager->addCluster(m_networkMetaStructure[x][y].second, cluster);
    return true;
}

/**
 * @brief NetworkInitializer::addNeighbors
 * @param x
 * @param y
 * @param cluster
 * @return
 */
bool NetworkInitializer::addNeighbors(const uint32_t x, const uint32_t y, Cluster* cluster)
{
    for(uint32_t side = 0; side <= 9; side++)
    {
        Neighbor target;
        uint32_t targetX = 0;
        uint32_t targetY = 0;

        switch (side) {
        case 0:
            {
                targetX = x;
                if(y == 0) {
                    targetY = m_networkDimensions[1];
                } else {
                    targetY = y - 1;
                }
                break;
            }
        case 1:
            {
                targetX = (x + 1) % m_networkDimensions[0];
                targetY = y;
                break;
            }
        case 2:
            {
                targetX = (x + 1) % m_networkDimensions[0];
                targetY = (y + 1) % m_networkDimensions[1];
                break;
            }
        case 3:
            {
                targetX = x;
                targetY = (y + 1) % m_networkDimensions[1];
                break;
            }
        case 4:
            {
                if(x == 0) {
                    targetX = m_networkDimensions[0];
                } else {
                    targetX = x - 1;
                }
                targetY = (y + 1) % m_networkDimensions[1];
                break;
            }
        case 5:
            {
                if(x == 0) {
                    targetX = m_networkDimensions[0];
                } else {
                    targetX = x - 1;
                }
                targetY = y;
                break;
            }
        default:
            break;
        }

        target.targetClusterId = m_networkMetaStructure[targetX][targetY].second;
        target.neighborType = m_networkMetaStructure[targetX][targetY].first;
        cluster->addNeighbor(side, target);
    }
    return true;
}

/**
 * @brief NetworkInitializer::createAxons
 * @return
 */
bool NetworkInitializer::createAxons()
{
    return true;
}

}
