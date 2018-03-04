#include <settings/database.h>
#include <database/databaseconnection.h>
#include <settings/config.h>

namespace KyoukoMind
{

/**
 * @brief DatabaseController::DatabaseController
 * @param conf
 */
Database::Database(Config *conf)
{
    //TODO: db-name richtig setzen
    bool ok;
    QStringList dbData = conf->getDatabaseConnection(&ok);
    if(ok) {
        m_dbConnection = new Persistence::DatabaseConnection(dbData.at(0),
                                                             dbData.at(1),
                                                             dbData.at(2),
                                                             dbData.at(3),
                                                             dbData.at(4),
                                                             "db");
    }
    assert(m_dbConnection->isInit());

    initClusterDatabase();
}

/**
 * @brief DatabaseController::~DatabaseController
 */
Database::~Database()
{
    m_dbConnection->deleteLater();
}

/**
 * @brief DatabaseController::writeLogToDatabase
 * @param content
 * @return
 */
bool Database::writeLogToDatabase(const QString &content)
{
    //TODO
    return false;
}

/**
 * @brief DatabaseController::initClusterDatabase
 */
void Database::initClusterDatabase()
{
    QString command;
    command.sprintf("CREATE TABLE IF NOT EXISTS cluster ("
                    "id int primary key not null,"
                    "ip text);");

    QSqlQuery query;
    m_dbConnection->runQuery(&query, command);
}

/**
 * @brief DatabaseController::containsClusterId
 * @param clusterId
 * @return
 */
bool Database::containsClusterId(const quint32 clusterId)
{
    QString command;
    command.sprintf("SELECT id FROM cluster WHERE %i = id;", clusterId);

    QSqlQuery query;
    m_dbConnection->runQuery(&query, command);

    if(query.size() > 0)
    {
        return true;
    }
    return false;
}

/**
 * @brief DatabaseController::addCluster
 * @param clusterId
 * @param ip
 * @return
 */
bool Database::addCluster(const quint32 clusterId, const QString &ip)
{
    if(containsClusterId(clusterId)) {
        return false;
    }

    QString command;
    command.sprintf("INSERT INTO cluster (id, ip) VALUES (%i, %s);",
                    clusterId,
                    stringToDbValue(ip).toUtf8().constData());

    QSqlQuery query;
    m_dbConnection->runQuery(&query, command);

    return true;
}

/**
 * @brief DatabaseController::getClusterIp
 * @param clusterId
 * @return
 */
QString Database::getClusterIp(const quint32 clusterId)
{
    QString ip = "";
    if(!containsClusterId(clusterId)) {
        return ip;
    }

    QString command;
    command.sprintf("SELECT ip FROM cluster WHERE %i = id;", clusterId);

    QSqlQuery query;
    m_dbConnection->runQuery(&query, command);

    if(query.size() > 0)
    {
        ip = query.value(0).toString();
    }

    return ip;
}

/**
 * @brief DatabaseController::stringToDbValue
 * @param value
 * @return
 */
QString Database::stringToDbValue(const QString &value) const
{
    if (value.isNull()) {
        return "NULL";
    }

    return QString("'%1'").arg(value);
}

}
