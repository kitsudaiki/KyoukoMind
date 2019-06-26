/**
 *  @file    config.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CONFIG
#define CONFIG

#include <common.h>

namespace KyoukoMind
{

class Config
{
public:
    Config();
    std::string getDirectoryPath() const;
    std::string getInitialFilePath() const;
};

}

#endif // CONFIG_H

