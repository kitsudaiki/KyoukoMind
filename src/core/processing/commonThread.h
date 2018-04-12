/**
 *  @file    commonThread.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef COMMONTHREAD_H
#define COMMONTHREAD_H

#include <common.h>

namespace KyoukoMind
{

class CommonThread
{
public:
    CommonThread();
    ~CommonThread();

    bool start();
    bool stop();

    void continueThread();
    void initBlockThread();

    bool isActive() const;

    virtual void run() = 0;

protected:
    std::thread* m_thread = nullptr;
    bool m_abort = false;
    bool m_block = false;
    bool m_active = false;

    std::mutex m_mutex;
    std::condition_variable m_cv;

    void blockThread();
    void sleepThread(const uint32_t seconrs);
};

}

#endif // COMMONTHREAD_H
