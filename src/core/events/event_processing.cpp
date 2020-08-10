/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "event_processing.h"

#include <core/events/events.h>
#include <kyouko_root.h>
#include <core/object_handling/segment.h>
#include <core/obj_converter.h>

namespace KyoukoMind
{

EventProcessing::EventProcessing()
{

}

void
process_getMetadata_event(GetMetadataEvent &event)
{
    DataItem* result = KyoukoRoot::m_segment->getMetadata();
    event.finishEvent(result->toString());
    delete result;
}

void
process_getObjSnapshot_event(GetObjSnapshotEvent &event)
{
    std::string convertedString = "";
    convertNetworkToString(convertedString);
    event.finishEvent(convertedString);
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

        switch(kyoukoEvent->getType())
        {
            case KyoukoEvent::GET_METADATA_EVENT:
                process_getMetadata_event(*dynamic_cast<GetMetadataEvent*>(kyoukoEvent));
                break;
            case KyoukoEvent::GET_OBJ_SNAPSHOT_EVENT:
                process_getObjSnapshot_event(*dynamic_cast<GetObjSnapshotEvent*>(kyoukoEvent));
                break;
            default:
                // TODO: error-log
                break;
        }
    }
}

}
