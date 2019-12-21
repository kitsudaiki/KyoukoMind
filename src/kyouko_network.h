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

#include <core/bricks/brick_handler.h>
#include <core/messaging/message_block_buffer.h>
#include <core/bricks/global_values_handler.h>

namespace KyoukoMind
{
class NetworkManager;
class BrickHandler;

class KyoukoNetwork
{

public:
    KyoukoNetwork();

    static KyoukoMind::Config* m_config;
    static KyoukoMind::BrickHandler* m_brickHandler;
    static KyoukoMind::GlobalValuesHandler* m_globalValuesHandler;
    static KyoukoMind::MessageBlockBuffer* m_messageBuffer;

    void start();

private:
    NetworkManager* m_networkManager = nullptr;
};

} // namespace KyoukoMind

#endif //KYOUKO_NETWORK_H
