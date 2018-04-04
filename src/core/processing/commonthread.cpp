#include "commonthread.h"

namespace KyoukoMind
{

/**
 * @brief CommonThread::CommonThread
 */
CommonThread::CommonThread()
{

}

/**
 * @brief CommonThread::~CommonThread
 */
CommonThread::~CommonThread()
{
    stop();
}

/**
 * @brief CommonThread::start
 * @return
 */
bool CommonThread::start()
{
    if(m_active) {
        return false;
    }
    m_thread = new std::thread(&CommonThread::run, this);
    return true;
}

/**
 * @brief CommonThread::stop
 * @return
 */
bool CommonThread::stop()
{
    if(!m_active) {
        return false;
    }
    m_thread->join();
    return true;
}

/**
 * @brief CommonThread::isActive
 * @return
 */
bool CommonThread::isActive() const
{
    return m_active;
}

}
