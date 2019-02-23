/**
 *  @file    demoUserInput.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include "demoUserInput.h"
#include "demoIO.h"
#include <core/clustering/clusterHandler.h>

namespace KyoukoMind
{

/**
 * @brief DemoUserInput::DemoUserInput
 */
DemoUserInput::DemoUserInput(ClusterHandler *clusterHandler)
{
    m_dataio = new DemoIO(clusterHandler);
    m_dataio->start();
}

/**
 * @brief DemoUserInput::inputLoop
 */
void
DemoUserInput::inputLoop()
{
    while(true)
    {
        std::string input;
        OUTPUT("wait for input")
        std::cin>>input;

        OUTPUT("input: ")
        OUTPUT(input)

        const char* charArray = input.c_str();
        for(int i = 0; i < input.size(); i++)
        {
            if(charArray[i] == '-') {
                m_dataio->sendNegative();
            }
            else if(charArray[i] == '+') {
                m_dataio->sendPositive();
            }
            else {
                m_dataio->sendOutData(charArray[i]);
                usleep(PROCESS_INTERVAL);
            }
        }
        usleep(PROCESS_INTERVAL);
    }
}

}
