/**
 * @file        callbacks.h
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

#ifndef KYOUKOMIND_CALLBACKS_H
#define KYOUKOMIND_CALLBACKS_H

#include <kyouko_root.h>

#include <libKitsunemimiHanamiMessaging/messaging_controller.h>
#include <libKitsunemimiHanamiMessaging/messaging_client.h>

#include <libKitsunemimiCommon/logger.h>

/**
 * @brief clientDataCallback
 * @param data
 * @param dataSize
 */
void
clientDataCallback(Kitsunemimi::Sakura::Session*,
                   const void* data,
                   const uint64_t dataSize)
{
    const char* charData = static_cast<const char*>(data);
    const std::string text(charData, dataSize);
    LOG_WARNING("client-text: " + text);
}

/**
 * @brief monitoringDataCallback
 */
void
monitoringDataCallback(Kitsunemimi::Sakura::Session*,
                       const void*,
                       const uint64_t)
{
}

/**
 * @brief sessionCallback
 * @param target
 * @param isInit
 * @param session
 * @param identifier
 */
void
sessionCreateCallback(Kitsunemimi::Hanami::MessagingClient* session,
                      const std::string identifier)
{
}

void
sessionCloseCallback(const std::string identifier)
{
}

#endif // KYOUKOMIND_CALLBACKS_H
