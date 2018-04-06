#include "networkInitializer.h"
#include <KyoChanNetwork.h>

#include <core/initializing/clusterinitilizer.h>
#include <core/initializing/axoninitializer.h>

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
                                       ClusterHandler *clusterHandler,
                                       MessageController *messageController)
{
    m_filePath = filePath;
    m_directoryPath = directoryPath;
    m_clusterHandler = clusterHandler;
    m_messageController = messageController;
}

/**
 * @brief NetworkInitializer::initNetwork
 * @return
 */
bool NetworkInitializer::initNetwork()
{
    // init randomizer
    srand(time(NULL));

    // check if values are valid
    if(m_clusterHandler == nullptr
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

    ClusterInitilizer clusterInit(&m_networkMetaStructure,
                                  m_networkDimensions[0],
                                  m_networkDimensions[1],
                                  m_directoryPath,
                                  m_clusterHandler,
                                  m_messageController);
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
        std::vector<MetaDataEntry> newLine;
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
            MetaDataEntry tempEntry;
            tempEntry.type = std::stoi(splittedLine[linePartNumber]);
            tempEntry.clusterId = m_idCounter;

            m_networkMetaStructure[lineNumber].push_back(tempEntry);
            m_idCounter++;
        }
    }

    // save network-dimensions
    m_networkDimensions[0] = allLines.size();
    m_networkDimensions[1] = firstLineLenght;

    return true;
}

}
