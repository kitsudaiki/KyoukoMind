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
    EMPTYCLUSTER = 1,
    EDGECLUSTER = 2,
    NODECLUSTER = 3
};

enum MessageType
{
    UNDEFINED = 0,
    DATAMESSAGE = 1,
    REPLYMESSAGE = 2,
    LEARNINGMESSAGE = 3,
    LEARNINGREPLYMESSAGE = 4,
    CYCLEFINISHMESSAGE = 5
};

#endif // ENUMS_H
