#ifndef KYSANETWORK_H
#define KYSANETWORK_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QHash>
#include <QFile>
#include <link.h>
#include <log/logger.h>
#include <settings/config.h>
#include <settings/database.h>


namespace KyoukoMind
{

class NetworkManager;

class KyoukoNetwork : public QObject
{
    Q_OBJECT

public:
    KyoukoNetwork(const QString &configPath);

    static KyoukoMind::Config *m_config;
    static Persistence::Logger *m_logger;
    static KyoukoMind::Database *m_db;

private:
    NetworkManager *m_netThreadManager = nullptr;

    bool initLogger(bool *ok);
};

}

#endif //KYSANETWORK_H
