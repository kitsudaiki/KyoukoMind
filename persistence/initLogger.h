#ifndef INITLOGGER_H
#define INITLOGGER_H

#include "KyoChanNetwork_global.h"

namespace Persistence {
class Logger;
}

namespace KyoChan_Network
{

class KyoChanNetworkSHARED_EXPORT InitLogger
{
public:
    InitLogger();
    Persistence::Logger *initLogger(bool *ok);

};

}

#endif // INITLOGGER_H
