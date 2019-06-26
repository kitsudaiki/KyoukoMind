/**
 *  @file    config.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <settings/config.h>

namespace KyoukoMind
{

Config::Config()
{
}

std::string
Config::getDirectoryPath() const
{
    // TODO
    return "/tmp/test/";
}

std::string
Config::getInitialFilePath() const
{
    // TODO
    return "/home/neptune/Schreibtisch/Projekte/KyoukoMind/test_cluster2";
}

}
