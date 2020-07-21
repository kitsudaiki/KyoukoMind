/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef GATEWAY_CONFIG_H
#define GATEWAY_CONFIG_H

#include <libKitsunemimiConfig/config_handler.h>

namespace KyoukoMind
{

void
registerConfigs()
{
    REGISTER_INT_CONFIG("Network", "port", 2448);

    REGISTER_STRING_CONFIG("Init", "file", "");

    REGISTER_STRING_CONFIG("Storage", "path", "/tmp/KyoukoMind/");
}

}

#endif // GATEWAY_CONFIG_H
