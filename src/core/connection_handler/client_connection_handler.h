/**
 * @file        client_connection_handler.h
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

#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <common.h>

namespace Kitsunemimi {
namespace Sakura {
class MessagingClient;
}
}

using Kitsunemimi::Sakura::MessagingClient;

class ClientConnectionHandler
{
public:
    ClientConnectionHandler();
    ~ClientConnectionHandler();

    bool sendToClient(const std::string &text);

    bool insertInput(const std::string &inputData);

    void setClientSession(MessagingClient* session);
    MessagingClient* getClientSession();

    uint32_t registerInput(const uint32_t brickId);
    uint32_t registerOutput(const uint32_t brickId);

private:
    std::atomic_flag m_clientSession_lock = ATOMIC_FLAG_INIT;
    MessagingClient* m_client = nullptr;
    uint32_t m_inputBrick = 0;
    uint32_t m_outputBrick = 0;
};

#endif // CLIENT_HANDLER_H
