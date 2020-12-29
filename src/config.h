/**
 * @file        config.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef KYOUKOMIND_CONFIG_H
#define KYOUKOMIND_CONFIG_H

#include <libKitsunemimiConfig/config_handler.h>

void
registerConfigs()
{
    REGISTER_STRING_CONFIG("DEFAULT", "sakura-file-locaion", "/etc/KyoukoMind/sakura-files");

    REGISTER_STRING_CONFIG("Init", "file", "/tmp/KyoukoMind_init");
}

#endif // KYOUKOMIND_CONFIG_H
