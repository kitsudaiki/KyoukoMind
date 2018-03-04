#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QVector>
#include <QStringList>
#include <assert.h>

namespace Persistence {
class DatabaseConnection;
}

namespace KyoukoMind
{

class Config;

class Database
{
public:
    Database(Config *conf);
    ~Database();

    bool writeLogToDatabase(const QString &content);

    void initClusterDatabase();
    bool containsClusterId(const quint32 clusterId);
    bool addCluster(const quint32 clusterId, const QString &ip);
    QString getClusterIp(const quint32 clusterId);

private:
    Persistence::DatabaseConnection* m_dbConnection = nullptr;

    QString stringToDbValue(const QString &value) const;
};

}

#endif // DATABASE_H
