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

    bool isActive() const;

    virtual void run() = 0;

protected:
    std::thread* m_thread = nullptr;
    bool m_abort = false;
    bool m_active = false;
};

}

#endif // COMMONTHREAD_H
