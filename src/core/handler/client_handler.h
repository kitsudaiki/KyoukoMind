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

struct InputObj
{
    uint32_t position = 0;
    uint32_t range = 0;
    float value = 0.0f;

    InputObj() {}

    InputObj(const uint32_t position, const uint32_t range)
    {
        this->position = position;
        this->range = range;
    }
};

using Kitsunemimi::Sakura::MessagingClient;

class ClientHandler
{
public:
    ClientHandler();
    ~ClientHandler();

    bool sendToClient();
    bool sendToMonitoring();
    bool sendToMonitoring(const char* data, const uint64_t dataSize);

    void setClientSession(MessagingClient* session);
    void setMonitoringSession(MessagingClient* session);

    MessagingClient* getClientSession();
    MessagingClient* getMonitoringSession();

    bool insertInput(const std::string &inputData);
    uint32_t registerInput(const uint32_t pos, const uint32_t range);
    InputObj getInput(const uint32_t pos);

    uint32_t registerOutput();
    void setOutput(const uint32_t pos, const float value);

private:
    std::atomic_flag m_clientSession_lock = ATOMIC_FLAG_INIT;
    MessagingClient* m_client = nullptr;

    std::atomic_flag m_monitoringSession_lock = ATOMIC_FLAG_INIT;
    MessagingClient* m_monitoring = nullptr;


    std::atomic_flag m_output_lock = ATOMIC_FLAG_INIT;
    std::vector<float> m_outputs;

    std::atomic_flag m_input_lock = ATOMIC_FLAG_INIT;
    std::vector<InputObj> m_inputs;
};

#endif // CLIENT_HANDLER_H
