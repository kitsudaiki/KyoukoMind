/**
 *  @file    enums.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */


#ifndef ENUMS_H
#define ENUMS_H

enum ClusterType
{
    UNDEFINED_CLUSTER = 0,
    EMPTY_CLUSTER = 1,
    EDGE_CLUSTER = 2,
    NODE_CLUSTER = 3
};

enum MessageType
{
    UNDEFINED_MESSAGE = 0,
    DATA_MESSAGE = 1,
    REPLY_MESSAGE = 2,
};

enum ContainerType
{
    UNDEFINED_CONTAINER = 0,
    EDGE_FOREWARD_CONTAINER = 1,
    AXON_EDGE_CONTAINER = 2,
    PENDING_EDGE_CONTAINER = 3,
    LEARNING_CONTAINER = 4,
    LEARNING_REPLY_CONTAINER = 5,
};

#endif // ENUMS_H
