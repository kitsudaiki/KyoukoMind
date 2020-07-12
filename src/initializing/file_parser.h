/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include <common.h>
#include <initializing/init_meta_data.h>

namespace KyoukoMind
{

const std::string readFile(const std::string filePath);

uint32_t parse2dTestfile(const std::string &fileContent,
                         std::vector<std::vector<InitMetaDataEntry>> &result);

}

#endif // FILE_PARSER_H
