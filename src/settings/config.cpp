#include <settings/config.h>
#include <config/configfileio.h>

namespace KyoukoMind
{

/**
 * @brief Config::Config
 * @param path path to the config-file
 */
Config::Config(const QString &path)
{
    m_configFile = new Persistence::ConfigFileIO(path);
}

/**
 * @brief Config::getNumberOfThreads
 * @param ok
 * @return
 */
int Config::getNumberOfThreads(bool *ok) const
{
    return m_configFile->getInt("number_of_processing_threads",
                                QStringList()<<"CPU",
                                ok);
}

/**
 * @brief Config::getNumberOfNodes
 * @param ok
 * @return
 */
int Config::getNumberOfNodes(bool *ok) const
{
    return m_configFile->getInt("number_of_nodes_per_cluster",
                                QStringList()<<"GENERAL",
                                ok);
}

/**
 * @brief Config::getPort
 * @param ok
 * @return
 */
int Config::getPort(bool *ok) const
{
    return m_configFile->getInt("port",
                                QStringList()<<"GENERAL",
                                ok);
}

/**
 * @brief Config::getDirectoryPath
 * @param ok
 * @return
 */
QString Config::getDirectoryPath(bool *ok) const
{
    return m_configFile->getEntry("directorypath",
                                  QStringList()<<"GENERAL",
                                  ok);
}

/**
 * @brief Config::getIpAdresses
 * @param ok
 * @return
 */
QStringList Config::getIpAdresses(bool *ok) const
{
    return m_configFile->getStringList("ips",
                                       QStringList()<<"GENERAL",
                                       ok);
}

/**
 * @brief Config::getDatabaseConnection get the connection-infos for the database
 * @param ok flag to check if method was successfull
 * @return list with all infos
 */
QStringList Config::getDatabaseConnection(bool *ok) const
{
    QStringList databaseCon;
    databaseCon.push_back(m_configFile->getEntry("type",
                                  QStringList()<<"DATABASE",
                                  ok));
    if(*ok == false) {
        return databaseCon;
    }
    databaseCon.push_back(m_configFile->getEntry("ip",
                                  QStringList()<<"DATABASE",
                                  ok));
    if(*ok == false) {
        return databaseCon;
    }
    databaseCon.push_back(m_configFile->getEntry("name",
                                  QStringList()<<"DATABASE",
                                  ok));
    if(*ok == false) {
        return databaseCon;
    }
    databaseCon.push_back(m_configFile->getEntry("user",
                                  QStringList()<<"DATABASE",
                                  ok));
    if(*ok == false) {
        return databaseCon;
    }
    databaseCon.push_back(m_configFile->getEntry("password",
                                  QStringList()<<"DATABASE",
                                  ok));
    return databaseCon;
}


/**
 * @brief Config::getLogFileDirPath
 * @param ok validation-flag
 * @return
 */
QString Config::getLogFileDirPath(bool *ok) const
{
    return m_configFile->getEntry("dictionary",
                                  QStringList()<<"LOG",
                                  ok);
}

/**
 * @brief Config::getLogFileName
 * @param ok validation-flag
 * @return
 */
QString Config::getLogFileName(bool *ok) const
{
    return m_configFile->getEntry("filename",
                                  QStringList()<<"LOG",
                                  ok);
}

/**
 * @brief Config::useFileForLogging
 * @param ok validation-flag
 * @return
 */
bool Config::useStdOutputForLogging(bool *ok) const
{
    return m_configFile->getBool("useStdOutput",
                                 QStringList()<<"LOG",
                                 ok);
}

/**
 * @brief Config::useFileForLogging
 * @param ok validation-flag
 * @return
 */
bool Config::useFileForLogging(bool *ok) const
{
    return m_configFile->getBool("useLogFile",
                                 QStringList()<<"LOG",
                                 ok);
}

/**
 * @brief Config::useDatabaseForLogging
 * @param ok validation-flag
 * @return
 */
bool Config::useDatabaseForLogging(bool *ok) const
{
    return m_configFile->getBool("useDatabase",
                                 QStringList()<<"LOG",
                                 ok);
}

/**
 * @brief Config::getLogLevels
 * @param ok validation-flag
 * @return
 */
QStringList Config::getLogLevels(bool *ok) const
{
    return m_configFile->getStringList("logLevel",
                                       QStringList()<<"LOG",
                                       ok);
}

}
