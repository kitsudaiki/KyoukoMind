/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef EVENT_PROCESSING_H
#define EVENT_PROCESSING_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>

namespace KyoukoMind
{

class EventProcessing
        : public Kitsunemimi::Thread
{
public:
    EventProcessing();

protected:
    void run();
};

}

#endif // EVENT_PROCESSING_H
