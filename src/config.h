#ifndef GATEWAY_CONFIG_H
#define GATEWAY_CONFIG_H

#include <libKitsunemimiConfig/config_handler.h>

namespace ToriiGateway
{

void
registerConfigs()
{
    REGISTER_INT_CONFIG("Torii", "port", 1337);

    REGISTER_STRING_CONFIG("Kyouko", "address", "127.0.0.1");
    REGISTER_INT_CONFIG("Kyouko", "port", 2448);
}

}

#endif // GATEWAY_CONFIG_H
