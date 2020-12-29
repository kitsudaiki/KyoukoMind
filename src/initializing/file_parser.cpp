/**
 * @file        file_parser.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "file_parser.h"

#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiCommon/common_methods/vector_methods.h>

#include <libKitsunemimiPersistence/logger/logger.h>

/**
 * @brief parse file to initialize a new network
 *
 * @param fileContent file to parse with the initial information
 * @param result reference for the result
 *
 * @return number of initialized bricks
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
