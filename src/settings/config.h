/**
 *  @file    config.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
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
    Config(const std::string &path);

    //general
    std::vector<std::string> getIpAdresses(bool *ok) const;
    int getNumberOfNodes(bool *ok) const;
    int getPort(bool *ok) const;
    std::string getDirectoryPath(bool *ok) const;
    std::string getInitialFilePath(bool *ok) const;

    //cpu
    int getNumberOfThreads(bool *ok) const;

    //log
    std::string getLogFileDirPath(bool *ok) const;
    std::string getLogFileName(bool *ok) const;
    bool useStdOutputForLogging(bool *ok) const;
    bool useFileForLogging(bool *ok) const;
    bool useDatabaseForLogging(bool *ok) const;
    std::vector<std::string> getLogLevels(bool *ok) const;

    //database
    std::vector<std::string> getDatabaseConnection(bool *ok) const;
private:
    //Persistence::ConfigFileIO* m_configFile = nullptr;
};

}

#endif // CONFIG_H

