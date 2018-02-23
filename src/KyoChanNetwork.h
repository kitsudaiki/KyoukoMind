#ifndef KYSANETWORK_H
#define KYSANETWORK_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QHash>
#include <link.h>
#include <persistence/config.h>
#include <log/logger.h>
#include <persistence/database.h>

namespace KyoChan_Network
{

class NetworkManager;

class KyoChanNetwork : public QObject
{
    Q_OBJECT

public:
    KyoChanNetwork(const QString &configPath);

    static KyoChan_Network::Config *m_config;
    static Persistence::Logger *m_logger;
    static KyoChan_Network::DatabaseController *m_dbCon;

private:
    NetworkManager *m_netThreadManager = nullptr;

    bool initLogger();
};

}

#endif //KYSANETWORK_H
