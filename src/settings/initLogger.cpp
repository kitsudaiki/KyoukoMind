#include <settings/initLogger.h>

#include <KyoChanNetwork.h>
#include <settings/config.h>
#include <log/logger.h>

namespace KyoChan_Network
{

InitLogger::InitLogger()
{

}

Persistence::Logger* InitLogger::initLogger(bool *ok)
{
    Persistence::Logger* logger = nullptr;

    //read config for standard-output
    bool useStdOutput = KyoChanNetwork::m_config->useStdOutputForLogging(ok);

    //read config for file-output
    bool useFileOutput = KyoChanNetwork::m_config->useFileForLogging(ok);
    QString dir = "";
    QString name = "";
    if(useFileOutput)
    {
        dir = KyoChanNetwork::m_config->getLogFileDirPath(ok);
        if(*ok == false) {
            return nullptr;
        }
        name = KyoChanNetwork::m_config->getLogFileName(ok);
        if(*ok == false) {
            return nullptr;
        }
    }

    //read config for database-output
    bool useDbOutput = KyoChanNetwork::m_config->useDatabaseForLogging(ok);
    QStringList databaseCon;
    if(useDbOutput)
    {
        databaseCon = KyoChanNetwork::m_config->getDatabaseConnection(ok);
        if(*ok == false)
        {
            return nullptr;
        }
    }

    //read config for log-levels
    QStringList logLevels;
    if(useStdOutput || useFileOutput || useDbOutput)
    {
        logLevels = KyoChanNetwork::m_config->getLogLevels(ok);
        if(*ok == false) {
            return nullptr;
        }
    }

    //create logger
    if(databaseCon.length() == 5)
    {
        logger = new Persistence::Logger(logLevels,
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
        logger = new Persistence::Logger(logLevels,
                                         useStdOutput,
                                         useFileOutput,
                                         dir,
                                         name,
                                         false);
    }
    return logger;
}

}
