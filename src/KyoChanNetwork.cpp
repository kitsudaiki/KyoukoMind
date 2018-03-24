#include <KyoChanNetwork.h>
#include <core/networkManager.h>
#include <core/cluster/clusterHandler.h>

namespace KyoukoMind
{

Config* KyoukoNetwork::m_config = nullptr;
KyoukoMind::Database* KyoukoNetwork::m_db = nullptr;

/**
 * @brief KyoChanNetwork::KyoChanNetwork
 * @param configPath
 */
KyoukoNetwork::KyoukoNetwork(const std::string &configPath)
{
    // config-file
    m_config = new Config(configPath);

    // logger
    bool ok = false;
    initLogger(&ok);

    // network-manager
    m_netThreadManager = new NetworkManager();
}

/**
 * @brief KyoukoNetwork::initLogger
 * @param ok
 * @return
 */
bool KyoukoNetwork::initLogger(bool *ok)
{
    // read config for standard-output
    bool useStdOutput = KyoukoNetwork::m_config->useStdOutputForLogging(ok);

    // read config for file-output
    bool useFileOutput = KyoukoNetwork::m_config->useFileForLogging(ok);
    std::string dir = "";
    std::string name = "";
    if(useFileOutput)
    {
        dir = KyoukoNetwork::m_config->getLogFileDirPath(ok);
        if(*ok == false) {
            return false;
        }
        name = KyoukoNetwork::m_config->getLogFileName(ok);
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
