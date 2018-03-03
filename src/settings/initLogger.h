#ifndef INITLOGGER_H
#define INITLOGGER_H

namespace Persistence {
class Logger;
}

namespace KyoChan_Network
{

class InitLogger
{
public:
    InitLogger();
    Persistence::Logger *initLogger(bool *ok);

};

}

#endif // INITLOGGER_H
