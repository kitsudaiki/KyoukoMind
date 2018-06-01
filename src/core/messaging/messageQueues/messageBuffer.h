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
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{
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
