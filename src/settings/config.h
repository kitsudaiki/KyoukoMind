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

namespace PerformanceIO {
//class ConfigFileIO;
}

namespace KyoukoMind
{

class Config
{
public:
    Config();

    //general
    std::vector<std::string> getIpAdresses() const;
    uint32_t getNumberOfNodes() const;
    int getPort() const;
    std::string getDirectoryPath() const;
    std::string getInitialFilePath() const;

    //cpu
    int getNumberOfThreads() const;

    //log
    std::string getLogFileDirPath() const;
    std::string getLogFileName() const;
    bool useStdOutputForLogging() const;
    bool useFileForLogging() const;
    bool useDatabaseForLogging() const;
    std::vector<std::string> getLogLevels() const;

    //database
    std::vector<std::string> getDatabaseConnection() const;
private:
    //Persistence::ConfigFileIO* m_configFile = nullptr;
};

}

#endif // CONFIG_H

