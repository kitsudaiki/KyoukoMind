/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef KYOUKOMIND_CONFIG_H
#define KYOUKOMIND_CONFIG_H

#include <libKitsunemimiConfig/config_handler.h>

void
registerConfigs()
{
    REGISTER_STRING_CONFIG("Init", "file", "/tmp/KyoukoMind_init");

    REGISTER_STRING_CONFIG("Network", "socket", "/tmp/KyoukoMind_uds.sock");
}

#endif // KYOUKOMIND_CONFIG_H
