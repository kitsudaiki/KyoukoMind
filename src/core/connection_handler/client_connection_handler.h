/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
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

    bool sendToClient();

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
