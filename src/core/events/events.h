/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef EVENTS_H
#define EVENTS_H

#include <common.h>
#include <libKitsunemimiCommon/threading/event.h>
#include <libKitsunemimiProjectNetwork/session.h>

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
        DOES_BRICK_ID_EXIST_EVENT = 3,
    };

    KyoukoEvent(Kitsunemimi::Project::Session* session,
                const uint64_t blockerId);
    ~KyoukoEvent();

    EventType getType() const;

    virtual bool processEvent() = 0;

protected:
    EventType m_type = UNDEFINED;
    Kitsunemimi::Project::Session* m_session = nullptr;
    uint64_t m_blockerId = 0;

    void finishEvent(const bool success,
                     const std::string &message = "");
};

//==================================================================================================
// GetMetadataEvent
//==================================================================================================
class GetMetadataEvent
        : public KyoukoEvent
{
public:
    GetMetadataEvent(Kitsunemimi::Project::Session* session,
                     const uint64_t blockerId);
    ~GetMetadataEvent();

    bool processEvent();
};

//==================================================================================================
// GetObjSnapshotEvent
//==================================================================================================
class GetObjSnapshotEvent
        : public KyoukoEvent
{
public:
    GetObjSnapshotEvent(Kitsunemimi::Project::Session* session,
                        const uint64_t blockerId);
    ~GetObjSnapshotEvent();

    bool processEvent();
};

//==================================================================================================
// DoesBrickIdExistEvent
//==================================================================================================
class DoesBrickIdExistEvent
        : public KyoukoEvent
{
public:
    DoesBrickIdExistEvent(Kitsunemimi::Project::Session* session,
                          const uint64_t blockerId);
    ~DoesBrickIdExistEvent();

    bool processEvent();

    uint32_t m_brickId = 0;
};

#endif // EVENTS_H
