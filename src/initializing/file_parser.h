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

inline std::vector<std::string>
splitString(const std::string &s, char delim)
{
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> tokens;
    while(std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

inline void
removeEmptyStrings(std::vector<std::string>& strings)
{
    std::vector<std::string>::iterator it = std::remove_if(
              strings.begin(),
              strings.end(),
              std::mem_fun_ref(&std::string::empty));
    // erase the removed elements
    strings.erase(it, strings.end());
}

namespace KyoukoMind
{

std::string
readFile(const std::string filePath);

bool
parse2dTestfile(const std::string fileContent,
                InitStructure* result);

}

#endif // FILE_PARSER_H