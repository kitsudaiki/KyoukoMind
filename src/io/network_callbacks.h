/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef NETWORK_CALLBACKS_H
#define NETWORK_CALLBACKS_H

#include <kyouko_root.h>
#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>
#include <libKitsunemimiPersistence/logger/logger.h>

namespace KyoukoMind
{

/**
 * @brief errorCallback
 */
void
errorCallback(void*,
              Kitsunemimi::Project::Session*,
              const uint8_t,
              const std::string message)
{
    LOG_ERROR("error-callback: " + message);
}

/**
 * @brief sessionCallback
 * @param target
 * @param isInit
 * @param session
 * @param sessionIdentifier
 */
void
sessionCallback(void* target,
                bool isInit,
                Kitsunemimi::Project::Session* session,
                const std::string sessionIdentifier)
{
    LOG_INFO("register incoming session with identifier: " + sessionIdentifier);

    KyoukoRoot* rootObject = static_cast<KyoukoRoot*>(target);
    if(isInit)
    {
        if(sessionIdentifier == "client") {
            rootObject->m_clientSession = session;
        }
        if(sessionIdentifier == "control") {
            rootObject->m_controlSession = session;
        }
        if(sessionIdentifier == "monitoring") {
            rootObject->m_monitoringSession = session;
        }
    }
    else
    {
        if(session->m_sessionIdentifier == "client")
        {
            delete rootObject->m_clientSession;
            rootObject->m_clientSession = nullptr;
        }
        if(session->m_sessionIdentifier == "control")
        {
            delete rootObject->m_controlSession;
            rootObject->m_controlSession = nullptr;
        }
        if(session->m_sessionIdentifier == "monitoring")
        {
            delete rootObject->m_monitoringSession;
            rootObject->m_clientSession = nullptr;
        }
    }
}

}

#endif // NETWORK_CALLBACKS_H
