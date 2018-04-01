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

    // split string
    std::vector<std::string> allLines = splitString(string_content, '\n');

    // read the single lines
    for(uint32_t lineNumber = 0; lineNumber < allLines.size(); lineNumber++)
    {
        // erase whitespaces
        allLines[lineNumber].erase(std::remove_if(allLines[lineNumber].begin(),
                                                  allLines[lineNumber].end(),
                                                  isspace),
                                   allLines[lineNumber].end());

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
    addNeighbors(x, y, cluster);
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

        std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

        target.targetClusterId = m_networkMetaStructure[next.first][next.second].second;
        target.neighborType = m_networkMetaStructure[next.first][next.second].first;
        target.distantToNextNodeCluster = getDistantToNextNodeCluster(x, y, side);

        cluster->addNeighbor(side, target);
    }
    return true;
}


/**
 * @brief NetworkInitializer::getDistantToNextNodeCluster
 * @param x
 * @param y
 * @param side
 * @return
 */
uint32_t NetworkInitializer::getDistantToNextNodeCluster(const uint32_t x,
                                                         const uint32_t y,
                                                         const uint8_t side)
{
    std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

    // TODO: max abort-distance
    for(uint32_t distance = 1; distance < m_networkMetaStructure.size(); distance++)
    {
        if(m_networkMetaStructure[next.first][next.second].first == (uint8_t)NODECLUSTER) {
            return distance;
        }
        if(m_networkMetaStructure[next.first][next.second].first == (uint8_t)EMPTYCLUSTER) {
            return 0;
        }
        next = getNext(next.first, next.second, side);
    }
    return 0;
}

/**
 * @brief NetworkInitializer::getNext
 * @param x
 * @param y
 * @param side
 * @return
 */
std::pair<uint32_t, uint32_t> NetworkInitializer::getNext(const uint32_t x,
                                                          const uint32_t y,
                                                          const uint8_t side)
{
    std::pair<uint32_t, uint32_t> result;
    switch (side) {
    case 0:
        {
            result.first = x;
            if(y == 0) {
                result.second = m_networkDimensions[1] - 1;
            } else {
                result.second = y - 1;
            }
            break;
        }
    case 1:
        {
            result.first = (x + 1) % m_networkDimensions[0];
            if(y % 2 == 1) {
                result.second = y;
            } else {
                if(y == 0) {
                    result.second = m_networkDimensions[1] - 1;
                } else {
                    result.second = y - 1;
                }
            }
            break;
        }
    case 2:
        {
            result.first = (x + 1) % m_networkDimensions[0];
            if(y % 2 == 1) {
                result.second = (y + 1) % m_networkDimensions[1];
            } else {
                result.second = y;
            }
            break;
        }
    case 3:
        {
            result.first = x;
            result.second = (y + 1) % m_networkDimensions[1];
            break;
        }
    case 4:
        {
            if(x == 0) {
                result.first = m_networkDimensions[0] - 1;
            } else {
                result.first = x - 1;
            }
            if(y % 2 == 1) {
                result.second = (y + 1) % m_networkDimensions[1];
            } else {
                result.second = y;
            }
            break;
        }
    case 5:
        {
            if(x == 0) {
                result.first = m_networkDimensions[0] - 1;
            } else {
                result.first = x - 1;
            }
            if(y % 2 == 1) {
                result.second = y;
            } else {
                if(y == 0) {
                    result.second = m_networkDimensions[1] - 1;
                } else {
                    result.second = y - 1;
                }
            }
            break;
        }
    default:
        break;
    }
    return result;
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
