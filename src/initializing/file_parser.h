/**
 *  @file    file_parser.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include <common.h>
#include <initializing/init_meta_data.h>

namespace KyoukoMind
{

const std::string readFile(const std::string filePath);

bool parse2dTestfile(const std::string &fileContent,
                     InitStructure &result);

}

#endif // FILE_PARSER_H
