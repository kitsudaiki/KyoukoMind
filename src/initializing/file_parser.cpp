/**
 *  @file    file_parser.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "file_parser.h"

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/common_methods/vector_methods.h>

namespace KyoukoMind
{

/**
 * @brief readFile
 * @param filePath
 * @return
 */
const std::string
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
 * @brief parse2dTestfile
 * @param fileContent
 * @param result
 * @return
 */
bool
parse2dTestfile(const std::string &fileContent,
                InitStructure &result)
{
    BrickID idCounter = 0;
    //InitStructure m_networkMetaStructure;
    uint32_t firstLineLenght = 0;

    // split string
    std::vector<std::string> allLines;
    Kitsunemimi::splitStringByDelimiter(allLines, fileContent, '\n');

    // read the single lines
    for(uint32_t lineNumber = 0; lineNumber < allLines.size(); lineNumber++)
    {

        // erase whitespaces
        Kitsunemimi::removeWhitespaces(allLines[lineNumber]);

        // split line
        std::vector<std::string> splittedLine;
        Kitsunemimi::splitStringByDelimiter(splittedLine, allLines[lineNumber], '|');

        // remove empty entries from the list
        Kitsunemimi::removeEmptyStrings(splittedLine);

        // process the splitted line
        for(uint32_t linePartNumber = 0; linePartNumber < splittedLine.size(); linePartNumber++)
        {
            // add new line to meat-structure-vector
            if(lineNumber == 0)
            {
                firstLineLenght = splittedLine.size();
                for(uint32_t i = 0; i < splittedLine.size(); i++)
                {
                    result.push_back(std::vector<InitMetaDataEntry>());
                }
            }

            if(firstLineLenght != splittedLine.size()) {
                assert(false);
            }

            InitMetaDataEntry tempEntry;
            tempEntry.type = std::stoi(splittedLine[linePartNumber]) + 1;
            tempEntry.brickId = idCounter;
            result[linePartNumber].push_back(tempEntry);

            idCounter++;
        }
    }

    return true;
}

} // namespace KyoukoMind
