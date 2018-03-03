#ifndef CONFIG
#define CONFIG

#include <QString>
#include <QVector>


namespace Persistence {
class ConfigFileIO;
}

namespace KyoukoMind
{

class Config
{
public:
    Config(const QString &path);

    //general
    QStringList getIpAdresses(bool *ok) const;
    int getNumberOfNodes(bool *ok) const;
    int getPort(bool *ok) const;
    QString getDirectoryPath(bool *ok) const;

    //cpu
    int getNumberOfThreads(bool *ok) const;

    //log
    QString getLogFileDirPath(bool *ok) const;
    QString getLogFileName(bool *ok) const;
    bool useStdOutputForLogging(bool *ok) const;
    bool useFileForLogging(bool *ok) const;
    bool useDatabaseForLogging(bool *ok) const;
    QStringList getLogLevels(bool *ok) const;

    //database
    QStringList getDatabaseConnection(bool *ok) const;
private:
    Persistence::ConfigFileIO* m_configFile = nullptr;
};

}

#endif // CONFIG_H

