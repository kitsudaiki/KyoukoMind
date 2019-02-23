/**
 *  @file    networkInitializer.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "networkInitializer.h"
#include <kyoChanNetwork.h>

#include <initializing/clusterInitilizer.h>
#include <initializing/axonInitializer.h>

namespace KyoukoMind
{

/**
 * @brief NetworkInitializer::NetworkInitializer
 * @param filePath
 * @param directoryPath
 * @param clusterManager
 */
NetworkInitializer::NetworkInitializer(const std::string fileContent,
                                       const std::string directoryPath,
                                       ClusterHandler *clusterHandler)
{
    m_fileContent = fileContent;
    m_directoryPath = directoryPath;
    m_clusterHandler = clusterHandler;
}

/**
 * @brief NetworkInitializer::initNetwork
 * @return
 */
bool
NetworkInitializer::initNetwork()
{
    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(m_clusterHandler == nullptr
            || m_fileContent == ""
            || m_directoryPath == "")
    {
        return false;
    }

    m_idCounter = 1;

    if(getNetworkMetaStructure() == false) {
        return false;
    }

    ClusterInitilizer clusterInit(&m_networkMetaStructure,
                                  m_networkDimensions[0],
                                  m_networkDimensions[1],
                                  m_directoryPath,
                                  m_clusterHandler);
    if(clusterInit.createNetwork() == false) {
        return false;
    }

    AxonInitializer axonInit(&m_networkMetaStructure,
                             m_networkDimensions[0],
                             m_networkDimensions[1]);
    if(axonInit.createAxons() == false) {
        return false;
    }

    return true;
}

/**
 * @brief NetworkInitializer::getNetworkMetaStructure
 * @return
 */
bool
NetworkInitializer::getNetworkMetaStructure()
{
    uint32_t firstLineLenght = 0;

    // split string
    std::vector<std::string> allLines = splitString(m_fileContent, '\n');

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
        std::vector<InitMetaDataEntry> newLine;
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
            InitMetaDataEntry tempEntry;
            tempEntry.type = std::stoi(splittedLine[linePartNumber]) + 1;
            tempEntry.clusterId = m_idCounter;
            m_networkMetaStructure[lineNumber].push_back(tempEntry);

            if(tempEntry.type == NODE_CLUSTER) {
                m_idCounter++;
            }
            m_idCounter++;
        }
    }

    // save network-dimensions
    m_networkDimensions[0] = allLines.size();
    m_networkDimensions[1] = firstLineLenght;

    return true;
}

}
