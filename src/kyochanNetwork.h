/**
 *  @file    kyochanNetwork.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef KYSANETWORK_H
#define KYSANETWORK_H

#include <common.h>
#include <settings/config.h>

namespace KyoukoMind
{
class NetworkManager;
class MessageController;

class KyoukoNetwork
{

public:
    KyoukoNetwork(const std::string &configPath);

    MessageController* getMessageController() const;

    static KyoukoMind::Config *m_config;

private:
    NetworkManager *m_networkManager = nullptr;

    bool initLogger(bool *ok);
};

}

#endif //KYSANETWORK_H
