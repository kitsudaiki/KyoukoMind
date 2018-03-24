#ifndef DATABASE_H
#define DATABASE_H

#include <string>
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

    bool writeLogToDatabase(const std::string &content);

    void initClusterDatabase();
    bool containsClusterId(const uint32_t clusterId);
    bool addCluster(const uint32_t clusterId, const std::string &ip);
    std::string getClusterIp(const uint32_t clusterId);

private:
    Persistence::DatabaseConnection* m_dbConnection = nullptr;

    std::string stringToDbValue(const std::string &value) const;
};

}

#endif // DATABASE_H
