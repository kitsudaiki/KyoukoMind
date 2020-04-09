#ifndef CONTROL_PROCESSING_H
#define CONTROL_PROCESSING_H

#include <root_object.h>
#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>
#include <libKitsunemimiPersistence/logger/logger.h>

namespace KyoukoMind
{

void
controlCallback(void* target,
                Kitsunemimi::Project::Session*,
                const uint64_t,
                DataBuffer* data)
{
    delete data;
}

}

#endif // CONTROL_PROCESSING_H
