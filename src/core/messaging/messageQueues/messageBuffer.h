/**
 *  @file    messageBuffer.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <common.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/learningMessages.h>

namespace KyoukoMind
{
class Message;
class DataMessage;
class DataAxonMessage;
class ReplyMessage;
class LearningMessage;
class LearningReplyMessage;

class MessageController;

class MessageBuffer
{
public:
    MessageBuffer(const ClusterID clusterId,
                 MessageController *controller);

protected:
    ClusterID m_clusterId;
    MessageController *m_controller = nullptr;

private:
};

}

#endif // MESSAGEQUEUE_H
