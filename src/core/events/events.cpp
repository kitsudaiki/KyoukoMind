/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "events.h"

#include <kyouko_root.h>
#include <core/object_handling/segment.h>
#include <core/obj_converter.h>

namespace KyoukoMind
{

//==================================================================================================
// KyoukoEvent
//==================================================================================================
KyoukoEvent::KyoukoEvent(Kitsunemimi::Project::Session* session,
                         const uint64_t blockerId)
{
    m_session = session;
    m_blockerId = blockerId;
}

KyoukoEvent::~KyoukoEvent()
{

}

KyoukoEvent::EventType
KyoukoEvent::getType() const
{
    return m_type;
}

void
KyoukoEvent::finishEvent(const std::string &message)
{
    m_session->sendResponse(message.c_str(), message.size(), m_blockerId);
}

//==================================================================================================
// GetMetadataEvent
//==================================================================================================
GetMetadataEvent::GetMetadataEvent(Kitsunemimi::Project::Session* session,
                                   const uint64_t blockerId)
    : KyoukoEvent(session, blockerId)
{
    m_type = GET_METADATA_EVENT;
}

GetMetadataEvent::~GetMetadataEvent()
{

}

bool
GetMetadataEvent::processEvent()
{
    DataItem* result = KyoukoRoot::m_segment->getMetadata();
    finishEvent(result->toString());
    delete result;

    return true;
}

//==================================================================================================
// GetObjSnapshotEvent
//==================================================================================================
GetObjSnapshotEvent::GetObjSnapshotEvent(Kitsunemimi::Project::Session* session,
                                         const uint64_t blockerId)
    : KyoukoEvent(session, blockerId)

{
    m_type = GET_OBJ_SNAPSHOT_EVENT;
}

GetObjSnapshotEvent::~GetObjSnapshotEvent()
{

}

bool
GetObjSnapshotEvent::processEvent()
{
    std::string convertedString = "";
    convertNetworkToString(convertedString);
    finishEvent(convertedString);

    return true;
}

}
