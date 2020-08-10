/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef EVENTS_H
#define EVENTS_H

#include <common.h>
#include <libKitsunemimiCommon/threading/event.h>
#include <libKitsunemimiProjectNetwork/session.h>

namespace KyoukoMind
{

//==================================================================================================
// KyoukoEvent
//==================================================================================================
class KyoukoEvent
        : public Kitsunemimi::Event
{
public:
    enum EventType {
        UNDEFINED = 0,
        GET_METADATA_EVENT = 1,
        GET_OBJ_SNAPSHOT_EVENT = 2,
    };

    KyoukoEvent(Kitsunemimi::Project::Session* session,
                const uint64_t blockerId);
    ~KyoukoEvent();

    EventType getType() const;

    void finishEvent(const std::string &message);

protected:
    EventType m_type = UNDEFINED;
    Kitsunemimi::Project::Session* m_session = nullptr;
    uint64_t m_blockerId = 0;
};

//==================================================================================================
// GetMetadataEvent
//==================================================================================================
class GetMetadataEvent
        : public KyoukoEvent
{
    GetMetadataEvent(Kitsunemimi::Project::Session* session,
                     const uint64_t blockerId);
    ~GetMetadataEvent();
};

//==================================================================================================
// GetObjSnapshotEvent
//==================================================================================================
class GetObjSnapshotEvent
        : public KyoukoEvent
{
    GetObjSnapshotEvent(Kitsunemimi::Project::Session* session,
                        const uint64_t blockerId);
    ~GetObjSnapshotEvent();
};

}

#endif // EVENTS_H
