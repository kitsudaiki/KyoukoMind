/**
 *  @file    config.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include <settings/config.h>
//#include <config/configfileio.h>

namespace KyoukoMind
{

/**
 * @brief Config::Config
 * @param path path to the config-file
 */
Config::Config(const std::string &path)
{
    //m_configFile = new Persistence::ConfigFileIO(path);
}

/**
 * @brief Config::getNumberOfThreads
 * @param ok
 * @return
 */
int Config::getNumberOfThreads(bool *ok) const
{
    return 1;
    //return m_configFile->getInt("number_of_processing_threads",
    //                            std::stringList()<<"CPU",
    //                            ok);
}

/**
 * @brief Config::getNumberOfNodes
 * @param ok
 * @return
 */
int Config::getNumberOfNodes(bool *ok) const
{
    return 10;
    //return m_configFile->getInt("number_of_nodes_per_cluster",
    //                            std::stringList()<<"GENERAL",
    //                            ok);
}

/**
 * @brief Config::getPort
 * @param ok
 * @return
 */
int Config::getPort(bool *ok) const
{
    return 1234;
    //return m_configFile->getInt("port",
    //                            std::stringList()<<"NETWORK",
    //                            ok);
}

/**
 * @brief Config::getDirectoryPath
 * @param ok
 * @return
 */
std::string Config::getDirectoryPath(bool *ok) const
{
    return "/tmp/test/";
    //return m_configFile->getEntry("directorypath",
    //                              std::stringList()<<"GENERAL",
    //                              ok);
}

/**
 * @brief Config::getInitialFilePath
 * @param ok
 * @return
 */
std::string Config::getInitialFilePath(bool *ok) const
{
    return "/home/neptune/Schreibtisch/Projekte/KyoukoMind/test_cluster";
    //return m_configFile->getEntry("initial_file_path",
    //                              std::stringList()<<"GENERAL",
    //                              ok);
}

/**
 * @brief Config::getIpAdresses
 * @param ok
 * @return
 */
std::vector<std::string> Config::getIpAdresses(bool *ok) const
{
    std::vector<std::string> ips;
    return ips;
    //return m_configFile->getStringList("ips",
    //                                   std::stringList()<<"NETWORK",
    //                                   ok);
}

/**
 * @brief Config::getDatabaseConnection get the connection-infos for the database
 * @param ok flag to check if method was successfull
 * @return list with all infos
 */
std::vector<std::string> Config::getDatabaseConnection(bool *ok) const
{
    std::vector<std::string> databaseCon;
    //databaseCon.push_back(m_configFile->getEntry("type",
    //                              std::stringList()<<"DATABASE",
    //                              ok));
    //if(*ok == false) {
    //    return databaseCon;
    //}
    //databaseCon.push_back(m_configFile->getEntry("ip",
    //                              std::stringList()<<"DATABASE",
    //                              ok));
    //databaseCon.push_back(m_configFile->getEntry("name",
    //                              std::stringList()<<"DATABASE",
    //                              ok));
    //databaseCon.push_back(m_configFile->getEntry("user",
    //                              std::stringList()<<"DATABASE",
    //                              ok));
    //databaseCon.push_back(m_configFile->getEntry("password",
    //                              std::stringList()<<"DATABASE",
    //                              ok));
    //*ok = true;
    return databaseCon;
}


/**
 * @brief Config::getLogFileDirPath
 * @param ok validation-flag
 * @return
 */
std::string Config::getLogFileDirPath(bool *ok) const
{
    return "/tmp/";
    //return m_configFile->getEntry("dictionary",
    //                              std::stringList()<<"LOG",
    //                              ok);
}

/**
 * @brief Config::getLogFileName
 * @param ok validation-flag
 * @return
 */
std::string Config::getLogFileName(bool *ok) const
{
    return "testlog";
    //return m_configFile->getEntry("filename",
    //                              std::stringList()<<"LOG",
    //                              ok);
}

/**
 * @brief Config::useFileForLogging
 * @param ok validation-flag
 * @return
 */
bool Config::useStdOutputForLogging(bool *ok) const
{
    return false;
    //return m_configFile->getBool("useStdOutput",
    //                             std::stringList()<<"LOG",
    //                             ok);
}

/**
 * @brief Config::useFileForLogging
 * @param ok validation-flag
 * @return
 */
bool Config::useFileForLogging(bool *ok) const
{
    return false;
    //return m_configFile->getBool("useLogFile",
    //                             std::stringList()<<"LOG",
    //                             ok);
}

/**
 * @brief Config::useDatabaseForLogging
 * @param ok validation-flag
 * @return
 */
bool Config::useDatabaseForLogging(bool *ok) const
{
    return false;
    //return m_configFile->getBool("useDatabase",
    //                             std::stringList()<<"LOG",
    //                             ok);
}

/**
 * @brief Config::getLogLevels
 * @param ok validation-flag
 * @return
 */
std::vector<std::string> Config::getLogLevels(bool *ok) const
{
    std::vector<std::string> loglevel;
    return loglevel;
    //return m_configFile->getStringList("logLevel",
    //                                   std::stringList()<<"LOG",
    //                                   ok);
}

}
