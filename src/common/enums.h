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
    UNDEFINEDCLUSTER = 0,
    EMPTY_CLUSTER = 1,
    EDGE_CLUSTER = 2,
    NODE_CLUSTER = 3
};

enum MessageType
{
    UNDEFINED = 0,
    DATA_MESSAGE = 1,
    DATA_AXON_MESSAGE = 1,
    REPLY_MESSAGE = 2,
    LEARNING_MESSAGE = 3,
    LEARNING_REPLY_MESSAGE = 4,
    CYCLE_FINISH_MESSAGE = 5
};

#endif // ENUMS_H
