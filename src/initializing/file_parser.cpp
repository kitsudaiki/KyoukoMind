/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "file_parser.h"

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/common_methods/vector_methods.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief parse2dTestfile
 * @param fileContent
 * @param result
 * @return
 */
uint32_t
parse2dTestfile(const std::string &fileContent,
                std::vector<std::vector<InitMetaDataEntry> > &result)
{
    uint32_t counter = 0;
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
        Kitsunemimi::removeEmptyStrings(splittedLine);

        result.push_back(std::vector<InitMetaDataEntry>());

        // process the splitted line
        for(uint32_t linePartNumber = 0; linePartNumber < splittedLine.size(); linePartNumber++)
        {
            // add new line to meat-structure-vector
            if(lineNumber == 0
                    && linePartNumber == 0)
            {
                firstLineLenght = static_cast<uint32_t>(splittedLine.size());
            }

            if(firstLineLenght != splittedLine.size()) {
                return 0;
            }

            InitMetaDataEntry tempEntry;
            tempEntry.type = static_cast<uint8_t>(std::stoi(splittedLine[linePartNumber]) + 1);
            result[lineNumber].push_back(tempEntry);
            counter++;
        }
    }

    return counter;
}
