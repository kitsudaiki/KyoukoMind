/**
 *  @file    kyouko_network.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef KYOUKO_NETWORK_H
#define KYOUKO_NETWORK_H

#include <common.h>

#include <settings/config.h>

#include <core/brick_handler.h>
#include <core/global_values_handler.h>

namespace KyoukoMind
{
class NetworkManager;
class BrickHandler;

class RootObject
{

public:
    RootObject();

    static KyoukoMind::Config* m_config;
    static KyoukoMind::BrickHandler* m_brickHandler;
    static KyoukoMind::GlobalValuesHandler* m_globalValuesHandler;

    void start();

private:
    NetworkManager* m_networkManager = nullptr;
};

} // namespace KyoukoMind

#endif //KYOUKO_NETWORK_H
