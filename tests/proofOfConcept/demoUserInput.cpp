/**
 *  @file    demoUserInput.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "demoUserInput.h"
#include "demoIO.h"
#include <core/messaging/messageController.h>
#include <core/clustering/clusterHandler.h>

namespace KyoukoMind
{

/**
 * @brief DemoUserInput::DemoUserInput
 */
DemoUserInput::DemoUserInput(MessageController *messageController,
                             ClusterHandler *clusterHandler)
{
    m_dataio = new DemoIO(messageController, clusterHandler);
    m_dataio->start();
}

/**
 * @brief DemoUserInput::inputLoop
 */
void DemoUserInput::inputLoop()
{
    while(true)
    {
        std::string input;
        OUTPUT("wait for input")
        std::cin>>input;

        OUTPUT("input: ")
        OUTPUT(input)

        const char* charArray = input.c_str();
        for(int i = 0; i < input.size(); i++) {
            usleep(PROCESS_INTERVAL);
            OUTPUT(charArray[i])
            m_dataio->sendOutData(charArray[i]);
        }
    }
}

}
