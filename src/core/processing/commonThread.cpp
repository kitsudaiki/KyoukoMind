/**
 *  @file    commonThread.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "commonThread.h"

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
    m_abort = false;
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
    m_abort = true;
    m_thread->join();
    return true;
}

/**
 * @brief CommonThread::continueThread
 */
void CommonThread::continueThread()
{
    {
        std::lock_guard<std::mutex> guard(m_mutex);
    }
    m_cv.notify_one();
}

/**
 * @brief CommonThread::initBlockThread
 */
void CommonThread::initBlockThread()
{
    m_block = true;
}

/**
 * @brief CommonThread::blockThread
 */
void CommonThread::blockThread()
{
    m_block = false;
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock);
    m_block = false;
}

/**
 * @brief CommonThread::sleepThread
 */
void CommonThread::sleepThread(const uint32_t seconrs)
{
    std::this_thread::sleep_for(chronoSec(seconrs));
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
