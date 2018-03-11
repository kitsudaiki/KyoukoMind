#ifndef ENUMS_H
#define ENUMS_H

enum ClusterType
{
    EMPTYCLUSTER = 0,
    EDGECLUSTER = 1,
    NODECLUSTER = 2
};

enum MessageType
{
    UNDEFINED = 0,
    DATAMESSAGE = 1,
    REPLYMESSAGE = 2,
    LEARNINGMESSAGE = 3,
    LEARNINGREPLYMESSAGE = 4
};

#endif // ENUMS_H
