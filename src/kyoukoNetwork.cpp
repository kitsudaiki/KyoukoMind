/**
 *  @file    kyoChanNetwork.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <kyoukoNetwork.h>
#include <core/networkManager.h>

namespace KyoukoMind
{

KyoukoMind::Config* KyoukoNetwork::m_config = nullptr;
Kitsune::Chan::Communication::MindClient* KyoukoNetwork::m_mindClient = nullptr;
KyoukoMind::BrickHandler* KyoukoNetwork::m_brickHandler = nullptr;
KyoukoMind::GlobalValuesHandler* KyoukoNetwork::m_globalValuesHandler = nullptr;
KyoukoMind::MessageBlockBuffer* KyoukoNetwork::m_internalMessageBuffer = nullptr;


/**
 * @brief Network::Network
 * @param configPath
 */
KyoukoNetwork::KyoukoNetwork()
{
    m_config = new Config();
    m_mindClient = new Kitsune::Chan::Communication::MindClient();
    m_mindClient->initConnection();
    m_brickHandler = new BrickHandler();
    m_globalValuesHandler = new GlobalValuesHandler();

    m_internalMessageBuffer = new MessageBlockBuffer();


    // logger
    bool ok = false;
    initLogger(&ok);
}

/**
 * @brief KyoukoNetwork::start
 */
void
KyoukoNetwork::start()
{
    // network-manager
    m_networkManager = new NetworkManager();
    m_networkManager->start();
}

/**
 * @brief KyoukoNetwork::getNodeManager
 * @return
 */
NetworkManager*
KyoukoNetwork::getNodeManager() const
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
