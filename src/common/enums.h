/**
 *  @file    enums.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
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

enum SectionStatus
{
    UNDEFINED_SECTION = 0,
    ACTIVE_SECTION = 1,
    DELETED_SECTION = 2,
};

enum ContainerType
{
    UNDEFINED_CONTAINER = 0,
    PENDING_EDGE_CONTAINER = 1,
    DIRECT_EDGE_CONTAINER = 2,
    FOREWARD_EDGE_CONTAINER = 3,
    AXON_EDGE_CONTAINER = 4,
    LEARNING_EDGE_CONTAINER = 5,
    LEARNING_REPLY_EDGE_CONTAINER = 6,
    STATUS_EDGE_CONTAINER = 7,
    DELETE_CONTAINER = 8,
    DEBUG_CONTAINER = 9,
};

#endif // ENUMS_H
