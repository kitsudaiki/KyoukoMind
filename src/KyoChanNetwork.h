#ifndef KYSANETWORK_H
#define KYSANETWORK_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QHash>
#include <link.h>
#include <log/logger.h>
#include <settings/config.h>
#include <settings/database.h>
#include <settings/initLogger.h>


namespace KyoukoMind
{

class NetworkManager;

class KyoChanNetwork : public QObject
{
    Q_OBJECT

public:
    KyoChanNetwork(const QString &configPath);

    static KyoukoMind::Config *m_config;
    static Persistence::Logger *m_logger;
    static KyoukoMind::DatabaseController *m_dbCon;

private:
    NetworkManager *m_netThreadManager = nullptr;

    bool initLogger();
};

}

#endif //KYSANETWORK_H
