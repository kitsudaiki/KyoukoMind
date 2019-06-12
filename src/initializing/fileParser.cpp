#include "fileParser.h"

namespace KyoukoMind
{

std::string
readFile(const std::string filePath)
{
    std::ifstream inFile;
    inFile.open(filePath);
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::string fileContent = strStream.str();
    inFile.close();
    return fileContent;
}

/**
 * @brief NetworkInitializer::getNetworkMetaStructure
 * @return
 */
bool
parse2dTestfile(const std::string fileContent,
                InitStructure* result)
{
    BrickID idCounter = 0;
    //InitStructure m_networkMetaStructure;
    uint32_t firstLineLenght = 0;

    // split string
    std::vector<std::string> allLines = splitString(fileContent, '\n');

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
        result->push_back(newLine);

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
            tempEntry.brickId = idCounter;
            (*result)[lineNumber].push_back(tempEntry);

            idCounter++;
        }
    }

    return true;
}

}
