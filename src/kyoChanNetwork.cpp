/**
 *  @file    kyoChanNetwork.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <kyoChanNetwork.h>
#include <core/networkManager.h>
#include <core/clustering/clusterHandler.h>

namespace KyoukoMind
{

Config* KyoukoNetwork::m_config = nullptr;
StatusReporter* KyoukoNetwork::m_reporter = nullptr;

/**
 * @brief KyoChanNetwork::KyoChanNetwork
 * @param configPath
 */
KyoukoNetwork::KyoukoNetwork()
{
    // config-file
    m_config = new Config();

    // reporter
    m_reporter = new StatusReporter();
    // TODO: get port-number from config
    m_reporter->initServer(1337);

    // logger
    bool ok = false;
    initLogger(&ok);

    // network-manager
    m_networkManager = new NetworkManager();
    m_networkManager->start();
}

/**
 * @brief KyoukoNetwork::getClusterHandler
 * @return
 */
ClusterHandler *KyoukoNetwork::getClusterHandler() const
{
    return m_networkManager->getClusterHandler();
}

/**
 * @brief KyoukoNetwork::getNodeManager
 * @return
 */
NetworkManager *KyoukoNetwork::getNodeManager() const
{
    return m_networkManager;
}

/**
 * @brief KyoukoNetwork::initLogger
 * @param ok
 * @return
 */
bool
KyoukoNetwork::initLogger(bool *ok)
{
    // read config for standard-output
    //bool useStdOutput = KyoukoNetwork::m_config->useStdOutputForLogging(ok);

    // read config for file-output
    bool useFileOutput = KyoukoNetwork::m_config->useFileForLogging();
    std::string dir = "";
    std::string name = "";
    if(useFileOutput)
    {
        dir = KyoukoNetwork::m_config->getLogFileDirPath();
        if(*ok == false) {
            return false;
        }
        name = KyoukoNetwork::m_config->getLogFileName();
        if(*ok == false) {
            return false;
        }
    }

    // read config for database-output
    /*bool useDbOutput = KyoukoNetwork::m_config->useDatabaseForLogging(ok);
    QStringList databaseCon;
    if(useDbOutput)
    {
        databaseCon = KyoukoNetwork::m_config->getDatabaseConnection(ok);
        if(*ok == false) {
            return false;
        }
    }*/

    // read config for log-levels
    /*QStringList logLevels;
    if(useStdOutput || useFileOutput || useDbOutput)
    {
        logLevels = KyoukoNetwork::m_config->getLogLevels(ok);
        if(*ok == false) {
            return false;
        }
    }*/

    // create logger
    /*if(databaseCon.length() == 5)
    {
        KyoukoNetwork::m_logger = new Persistence::Logger(logLevels,
                                                          useStdOutput,
                                                          useFileOutput,
                                                          dir,
                                                          name,
                                                          useDbOutput,
                                                          databaseCon.at(0),
                                                          databaseCon.at(1),
                                                          databaseCon.at(2),
                                                          databaseCon.at(3),
                                                          databaseCon.at(4));
    } else {
        KyoukoNetwork::m_logger = new Persistence::Logger(logLevels,
                                                          useStdOutput,
                                                          useFileOutput,
                                                          dir,
                                                          name,
                                                          false);
    }*/

    return true;
}

}
