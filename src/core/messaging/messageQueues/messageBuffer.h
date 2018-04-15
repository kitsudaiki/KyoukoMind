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
class ReplyMessage;

class Cluster;

class MessageController;

class MessageBuffer
{
public:
    MessageBuffer(Cluster *cluster,
                 MessageController *controller);

protected:
    Cluster* m_cluster;
    MessageController *m_controller = nullptr;

private:
};

}

#endif // MESSAGEQUEUE_H
