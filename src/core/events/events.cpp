/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "events.h"

#include <kyouko_root.h>
#include <core/object_handling/segment.h>
#include <core/obj_converter.h>

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
KyoukoEvent::finishEvent(const bool success,
                         const std::string &message)
{
    DataMap response;
    response.insert("success", new DataValue(success));
    if(message != "")
    {
        response.insert("message", new DataValue(message));
    }

    const std::string responseMessage = response.toString();
    m_session->sendResponse(responseMessage.c_str(), responseMessage.size(), m_blockerId);
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
    finishEvent(true, result->toString());
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
    finishEvent(true, convertedString);

    return true;
}

//==================================================================================================
// GetObjSnapshotEvent
//==================================================================================================
DoesBrickIdExistEvent::DoesBrickIdExistEvent(Kitsunemimi::Project::Session* session,
                                             const uint64_t blockerId)
    : KyoukoEvent(session, blockerId)

{
    m_type = DOES_BRICK_ID_EXIST_EVENT;
}

DoesBrickIdExistEvent::~DoesBrickIdExistEvent()
{

}

bool
DoesBrickIdExistEvent::processEvent()
{
    const bool result = KyoukoRoot::m_segment->bricks.numberOfItems < m_brickId;
    finishEvent(result);

    return true;
}

