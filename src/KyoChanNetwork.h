#ifndef KYSANETWORK_H
#define KYSANETWORK_H

#include <common.h>
#include <settings/config.h>

namespace KyoukoMind
{

class NetworkManager;

class KyoukoNetwork
{

public:
    KyoukoNetwork(const std::string &configPath);

    static KyoukoMind::Config *m_config;

private:
    NetworkManager *m_netThreadManager = nullptr;

    bool initLogger(bool *ok);
};

}

#endif //KYSANETWORK_H
