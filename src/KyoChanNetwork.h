#ifndef KYSANETWORK_H
#define KYSANETWORK_H

#include <QVector>
#include <string>
#include <QHash>
#include <QFile>
#include <link.h>
#include <settings/config.h>
#include <settings/database.h>


namespace KyoukoMind
{

class NetworkManager;

class KyoukoNetwork
{

public:
    KyoukoNetwork(const std::string &configPath);

    static KyoukoMind::Config *m_config;
    static KyoukoMind::Database *m_db;

private:
    NetworkManager *m_netThreadManager = nullptr;

    bool initLogger(bool *ok);
};

}

#endif //KYSANETWORK_H
