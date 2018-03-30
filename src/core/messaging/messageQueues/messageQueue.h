#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <common.h>

namespace KyoukoMind
{
class Message;
class DataMessage;
class ReplyMessage;
class LearningMessage;
class LearningReployMessage;

class MessageController;

class MessageQueue
{
public:
    MessageQueue(const ClusterID clusterId,
                 MessageController *controller);

protected:
    ClusterID m_clusterId;
    MessageController *m_controller = nullptr;

private:
};

}

#endif // MESSAGEQUEUE_H
