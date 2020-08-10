/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "event_processing.h"
#include <core/events/events.h>

EventProcessing::EventProcessing()
{

}

void
EventProcessing::run()
{
    while(!m_abort)
    {

        Kitsunemimi::Event* event = getEventFromQueue();

        if(event == nullptr)
        {
            sleepThread(10);
            continue;
        }

        KyoukoEvent* kyoukoEvent = dynamic_cast<KyoukoEvent*>(event);
        assert(kyoukoEvent != nullptr);

        kyoukoEvent->processEvent();
    }
}
